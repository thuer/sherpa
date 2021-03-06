#ifndef COMIX_Main_Comix_H
#define COMIX_Main_Comix_H

#include "COMIX/Main/Process_Group.H"
#include "PHASIC++/Main/Process_Integrator.H"
#include "PHASIC++/Process/ME_Generator_Base.H"
#include "ATOOLS/Org/My_MPI.H"

namespace MODEL  { class Model_Base;   }
namespace PDF    { class Remnant_Base; }

namespace COMIX {

  class Single_Process;
  class Cluster_Algorithm;

  class Comix: public Process_Group, public PHASIC::ME_Generator_Base {
  private :

    Cluster_Algorithm *p_cluster;

    std::vector<std::vector<Single_Process*> > m_umprocs;
    std::vector<PHASIC::Process_Base*>         m_rsprocs;

    std::string m_path, m_file;
    time_t m_mets;

    void PrintLogo(std::ostream &s);
    void PrintVertices();

  public :

    // constructor
    Comix();

    // destructor
    ~Comix();

    // member functions
    bool Initialize(const std::string &path,const std::string &file,
                    MODEL::Model_Base *const model,
                    BEAM::Beam_Spectra_Handler *const beamhandler,
                    PDF::ISR_Handler *const isrhandler);
    PHASIC::Process_Base *InitializeProcess(const PHASIC::Process_Info &pi,
                                            bool add);
    int PerformTests();
    bool NewLibraries();

    void SetClusterDefinitions(PDF::Cluster_Definitions_Base *const defs);

    void PreCluster(PHASIC::Process_Base *const proc,
		    const ATOOLS::Vec4D_Vector &p);

    ATOOLS::Cluster_Amplitude *ClusterConfiguration
    (PHASIC::Process_Base *const proc,const ATOOLS::Vec4D_Vector &p,
     const size_t &mode);

  }; // end of class Comix

} // end of namespace COMIX

#endif

#include "COMIX/Main/Single_Process.H"
#include "COMIX/Main/Single_Dipole_Term.H"
#include "PDF/Main/ISR_Handler.H"
#include "ATOOLS/Org/Run_Parameter.H"
#include "ATOOLS/Math/Random.H"
#include "ATOOLS/Org/MyStrStream.H"
#include "ATOOLS/Org/Data_Reader.H"
#include "MODEL/Main/Model_Base.H"
#include "PDF/Remnant/Remnant_Base.H"
#include "PHASIC++/Main/Phase_Space_Handler.H"
#include "COMIX/Cluster/Cluster_Algorithm.H"
#include "METOOLS/Explicit/Vertex.H"
#include "ATOOLS/Org/Shell_Tools.H"
#include "ATOOLS/Org/My_File.H"
#include "ATOOLS/Org/CXXFLAGS.H"

using namespace COMIX;
using namespace PHASIC;
using namespace MODEL;
using namespace ATOOLS;

Comix::Comix(): 
  ME_Generator_Base("Comix"), p_cluster(NULL)
{
}

Comix::~Comix() 
{
  if (p_cluster) delete p_cluster;
}

#define RED(ARG) om::red<<ARG<<om::reset
#define GREEN(ARG) om::green<<ARG<<om::reset
#define BLUE(ARG) om::blue<<ARG<<om::reset
#define YELLOW(ARG) om::brown<<ARG<<om::reset
#define BLACK(ARG) ARG

void Comix::PrintLogo(std::ostream &s)
{
  s<<"+----------------------------------+\n";
  s<<"|                                  |\n";
  s<<"|      "<<RED("CCC")<<"  "<<GREEN("OOO")<<"  "
   <<BLUE("M")<<"   "<<BLUE("M")<<" "<<BLACK("I")<<" "
   <<YELLOW("X")<<"   "<<YELLOW("X")<<"     |\n";
  s<<"|     "<<RED("C")<<"    "<<GREEN("O")<<"   "
   <<GREEN("O")<<" "<<BLUE("MM")<<" "<<BLUE("MM")
   <<" "<<BLACK("I")<<"  "<<YELLOW("X")<<" "
   <<YELLOW("X")<<"      |\n";
  s<<"|     "<<RED("C")<<"    "<<GREEN("O")
   <<"   "<<GREEN("O")<<" "<<BLUE("M")<<" "
   <<BLUE("M")<<" "<<BLUE("M")<<" "<<BLACK("I")
   <<"   "<<YELLOW("X")<<"       |\n";
  s<<"|     "<<RED("C")<<"    "<<GREEN("O")
   <<"   "<<GREEN("O")<<" "<<BLUE("M")<<"   "
   <<BLUE("M")<<" "<<BLACK("I")<<"  "
   <<YELLOW("X")<<" "<<YELLOW("X")<<"      |\n";
  s<<"|      "<<RED("CCC")<<"  "<<GREEN("OOO")
   <<"  "<<BLUE("M")<<"   "<<BLUE("M")<<" "
   <<BLACK("I")<<" "<<YELLOW("X")<<"   "
   <<YELLOW("X")<<"     |\n";
  s<<"|                                  |\n";
  s<<"+==================================+\n";
  s<<"|  Color dressed  Matrix Elements  |\n";
  s<<"|     http://comix.freacafe.de     |\n";
  s<<"|   please cite  JHEP12(2008)039   |\n";
  s<<"+----------------------------------+\n";
  rpa->gen.AddCitation
    (1,"Comix is published under \\cite{Gleisberg:2008fv}.");
}

void Comix::PrintVertices()
{
  if (msg_LevelIsDebugging()) {
    msg_Out()<<METHOD<<"(): {\n\n   Implemented currents:\n\n";
    Current_Getter::PrintGetterInfo(msg_Out(),10);
    msg_Out()<<"\n   Implemented lorentz calculators:\n\n";
    LC_Getter::PrintGetterInfo(msg_Out(),10);
    msg_Out()<<"\n   Implemented color calculators:\n\n";
    CC_Getter::PrintGetterInfo(msg_Out(),10);
    msg_Out()<<"\n}\n";
  }
}

bool Comix::Initialize(const std::string &path,const std::string &file,
		       MODEL::Model_Base *const model,
		       BEAM::Beam_Spectra_Handler *const beamhandler,
		       PDF::ISR_Handler *const isrhandler) 
{
  m_path=path;
  m_file=file;
  p_model=model;
  p_int->SetBeam(beamhandler); 
  p_int->SetISR(isrhandler);
  // init mapping file
  Data_Reader read(" ",";","!","=");
  read.AddComment("#");
  read.AddWordSeparator("\t");
  read.SetInputPath(m_path);
  read.SetInputFile(m_file);
  SetPSMasses(&read);
  PrintLogo(msg->Info());
  PrintVertices();
#ifdef USING__MPI
  if (MPI::COMM_WORLD.Get_rank()==0)
#endif
  MakeDir(rpa->gen.Variable("SHERPA_CPP_PATH")+"/Process",true);
  My_In_File::OpenDB
    (rpa->gen.Variable("SHERPA_CPP_PATH")+"/Process/Comix/");
  return true;
}

PHASIC::Process_Base *Comix::
InitializeProcess(const PHASIC::Process_Info &pi, bool add)
{
  if (p_model==NULL) return NULL;
  m_umprocs.push_back(std::vector<Single_Process*>());
  PHASIC::Process_Base *newxs(NULL);
  size_t nis(pi.m_ii.NExternal()), nfs(pi.m_fi.NExternal());
  bool oneisgroup(pi.m_ii.IsGroup()||pi.m_fi.IsGroup());
  std::map<std::string,std::string> pmap;
  if (oneisgroup) {
    newxs = new Process_Group();
    newxs->SetGenerator(this);
    newxs->Init(pi,p_int->Beam(),p_int->ISR());
    newxs->Get<COMIX::Process_Base>()->SetModel(p_model);
    if (!newxs->Get<Process_Group>()->Initialize(&pmap,&m_umprocs.back())) {
      msg_Debugging()<<METHOD<<"(): Init failed for '"
		     <<newxs->Name()<<"'\n";
      delete newxs;
      return NULL;
    }
    newxs->Integrator()->SetHelicityScheme(pi.m_hls);
    newxs->Get<COMIX::Process_Base>()->SetModel(p_model);
    newxs->Get<COMIX::Process_Base>()->SetGPath(pi.m_gpath);
    My_In_File::ExecDB
      (rpa->gen.Variable("SHERPA_CPP_PATH")+"/Process/Comix/","begin");
    if (!newxs->Get<PHASIC::Process_Group>()->ConstructProcesses()) {
      My_In_File::ExecDB
	(rpa->gen.Variable("SHERPA_CPP_PATH")+"/Process/Comix/","commit");
      msg_Debugging()<<METHOD<<"(): Construct failed for '"
		     <<newxs->Name()<<"'\n";
      delete newxs;
      return NULL;
    }
    My_In_File::ExecDB
      (rpa->gen.Variable("SHERPA_CPP_PATH")+"/Process/Comix/","commit");
    msg_Tracking()<<"Initialized '"<<newxs->Name()<<"'\n";
  }
  else {
    newxs = new Single_Process();
    newxs->SetGenerator(this);
    newxs->Init(pi,p_int->Beam(),p_int->ISR());
    newxs->Integrator()->SetHelicityScheme(pi.m_hls);
    newxs->Get<COMIX::Process_Base>()->SetModel(p_model);
    newxs->Get<COMIX::Process_Base>()->SetGPath(pi.m_gpath);
    My_In_File::ExecDB
      (rpa->gen.Variable("SHERPA_CPP_PATH")+"/Process/Comix/","begin");
    if (!newxs->Get<Single_Process>()->Initialize(&pmap,&m_umprocs.back())) {
      My_In_File::ExecDB
	(rpa->gen.Variable("SHERPA_CPP_PATH")+"/Process/Comix/","commit");
      msg_Debugging()<<METHOD<<"(): Init failed for '"
		     <<newxs->Name()<<"'\n";
      delete newxs;
      return NULL;
    }
    if (!newxs->Get<Single_Process>()->MapProcess())
      if (!msg_LevelIsTracking()) msg_Info()<<"."<<std::flush;
    My_In_File::ExecDB
      (rpa->gen.Variable("SHERPA_CPP_PATH")+"/Process/Comix/","commit");
  }
  if (add) Add(newxs);
  else m_rsprocs.push_back(newxs);
  return newxs;
}

int Comix::PerformTests()
{
  My_In_File::CloseDB
    (rpa->gen.Variable("SHERPA_CPP_PATH")+"/Process/Comix/");
  if (!Tests()) return 0;
  for (size_t i=0;i<m_rsprocs.size();++i)
    if (!m_rsprocs[i]->Get<COMIX::Process_Base>()->Tests()) return false;
  return 1;
}

bool Comix::NewLibraries()
{
  return false;
}

void Comix::SetClusterDefinitions(PDF::Cluster_Definitions_Base *const defs)
{
  if (p_cluster==NULL) p_cluster = new Cluster_Algorithm(this);
  p_cluster->SetClusterDefinitions(defs);
}

void Comix::PreCluster(PHASIC::Process_Base *const proc,const Vec4D_Vector &p)
{
  p_cluster->PreCluster(proc->Get<COMIX::Single_Process>(),
			proc->Get<COMIX::Single_Dipole_Term>(),p);
}

Cluster_Amplitude *Comix::ClusterConfiguration
(PHASIC::Process_Base *const proc,const Vec4D_Vector &p,
 const size_t &mode)
{
  p_cluster->Cluster(proc->Get<COMIX::Single_Process>(),
		     proc->Get<COMIX::Single_Dipole_Term>(),p,mode);
  return p_cluster->GetAmplitude();
}

DECLARE_GETTER(Comix,"Comix",ME_Generator_Base,ME_Generator_Key);

ME_Generator_Base *ATOOLS::Getter
<ME_Generator_Base,ME_Generator_Key,Comix>::
operator()(const ME_Generator_Key &key) const
{
  return new Comix();
}

void ATOOLS::Getter<ME_Generator_Base,ME_Generator_Key,Comix>::
PrintInfo(std::ostream &str,const size_t width) const
{ 
  str<<"The Comix ME generator"; 
}
