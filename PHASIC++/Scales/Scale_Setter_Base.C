#include "PHASIC++/Scales/Scale_Setter_Base.H"

#include "PHASIC++/Process/Process_Base.H"
#include "PHASIC++/Main/Process_Integrator.H"
#include "PHASIC++/Main/Phase_Space_Handler.H"
#include "PDF/Main/Cluster_Definitions_Base.H"
#include "PDF/Main/Shower_Base.H"
#include "ATOOLS/Org/Run_Parameter.H"
#include "ATOOLS/Org/Data_Reader.H"

#define COMPILE__Getter_Function
#define OBJECT_TYPE PHASIC::Scale_Setter_Base
#define PARAMETER_TYPE PHASIC::Scale_Setter_Arguments
#define EXACTMATCH false
#include "ATOOLS/Org/Getter_Function.C"

using namespace PHASIC;
using namespace MODEL;
using namespace ATOOLS;

Scale_Setter_Base::Scale_Setter_Base
(const Scale_Setter_Arguments &args):
  p_proc(args.p_proc), p_caller(p_proc),
  p_model(args.p_model), p_cpls(args.p_cpls), p_subs(NULL),
  m_scale(stp::size), m_coupling(args.m_coupling),
  m_nin(args.m_nin), m_nout(args.m_nout)
{
  for (size_t i(0);i<stp::size;++i) m_scale[i]=sqr(rpa->gen.Ecms());
  if (p_proc) {
    m_nin=p_proc->NIn();
    m_nout=p_proc->NOut();
  }
  m_p.resize(m_nin+m_nout);
}

void Scale_Setter_Base::SetCouplings()
{
  if (p_proc==NULL || p_proc->Integrator()->ISR()==NULL) return;
  DEBUG_FUNC(p_proc->Name());
  if (p_cpls==NULL) THROW(fatal_error,"No coupling information");
  p_subs=p_proc->GetSubevtList();
  Data_Reader read(" ",",","#",":");
  std::vector<std::vector<std::string> > helpsvv;
  read.SetAddCommandLine(false);
  read.SetString(m_coupling);
  read.MatrixFromString(helpsvv,"");
  for (size_t i(0);i<helpsvv.size();++i) {
    if (helpsvv[i].size()!=2) {
      if (helpsvv[i].size()==1 && helpsvv[i][0]=="None") break;
      THROW(fatal_error,"Invalid tag "+m_coupling+".");
    }
    Coupling_Map::iterator cit(p_cpls->lower_bound(helpsvv[i][0]));
    Coupling_Map::iterator eit(p_cpls->upper_bound(helpsvv[i][0]));
    if (cit!=eit) {
      int idx(ToType<int>(helpsvv[i][1]));
      if (idx<0) continue;
      if (idx>=m_scale.size())
	THROW(fatal_error,"Index too large for "+helpsvv[i][0]+".");
      for (;cit!=eit;++cit) {
	msg_Debugging()<<*cit->second<<" -> "<<helpsvv[i][1]<<"\n";
	if (cit->second->Sub()==NULL) cit->second->SetScale(&m_scale[idx]);
	else {
	  cit->second->Sub()->m_mu2.resize(m_scale.size());
	  cit->second->SetScale(&cit->second->Sub()->m_mu2[idx]);
	}
      }
    }
    else {
      msg_Error()<<METHOD<<"("<<p_proc->Name()<<"): Valid tags are\n ";
      for (Coupling_Map::const_iterator cit(p_cpls->begin());
	   cit!=p_cpls->end();++cit) msg_Error()<<" "<<cit->first;
      msg_Error()<<"\n";
      THROW(fatal_error,"Invalid coupling tag "+helpsvv[i][0]+".");
    }
  }
  m_fac.resize(2,1.0);
}

Scale_Setter_Base::~Scale_Setter_Base()
{
}

PDF::CParam Scale_Setter_Base::CoreScale(Cluster_Amplitude *const ampl) const
{
  return PDF::CParam(0.0);
}

void Scale_Setter_Base::ShowSyntax(const size_t i)
{
  if (!msg_LevelIsInfo() || i==0) return;
  msg_Out()<<METHOD<<"(): {\n\n"
	   <<"   // available scale choices\n\n";
  Scale_Getter_Function::PrintGetterInfo(msg->Out(),25);
  msg_Out()<<"\n}"<<std::endl;
}

double Scale_Setter_Base::HTM() const
{
  double htm(0.0);
  for (size_t i(m_nin);i<m_p.size();++i) htm+=m_p[i].MPerp();
  return htm;
}

double Scale_Setter_Base::HT() const
{
  double ht(0.0);
  for (size_t i(m_nin);i<m_p.size();++i) ht+=m_p[i].PPerp();
  return ht;
}

Vec4D Scale_Setter_Base::PSum() const
{
  Vec4D sum(0.0,0.0,0.0,0.0);
  for (size_t i(m_nin);i<m_p.size();++i) sum+=m_p[i];
  return sum;
}

double Scale_Setter_Base::BeamThrust() const
{
  double tauB(0.);
  for (size_t i(m_nin);i<m_p.size();++i) tauB+=m_p[i][0]-dabs(m_p[i][3]);
  return tauB;
}

void Scale_Setter_Base::PreCalc(const Vec4D_Vector &p,const size_t &mode)
{
}

double Scale_Setter_Base::CalculateScale
(const ATOOLS::Vec4D_Vector &p,const size_t mode)
{
  DEBUG_FUNC((p_proc?p_proc->Name():""));
  if (!m_escale.empty()) {
    for (size_t i(0);i<m_escale.size();++i) m_scale[i]=m_escale[i];
    while (m_ampls.size()) {
      m_ampls.back()->Delete();
      m_ampls.pop_back();
    }
    if (p_subs) {
      for (size_t i(0);i<p_subs->size();++i) {
	NLO_subevt *sub((*p_subs)[i]);
	size_t ssz(Min(sub->m_mu2.size(),m_scale.size()));
	for (size_t j(0);j<ssz;++j) sub->m_mu2[j]=m_scale[j];
	if (sub->p_ampl) {
	  sub->p_ampl->Delete();
	  sub->p_ampl=NULL;
	}
      }
    }
    p_cpls->Calculate();
    return m_scale[stp::fac];    
  }
  if (p_subs==NULL) {
    m_p.resize(p.size());
    for (size_t j(0);j<m_p.size();++j) m_p[j]=p[j];
    PreCalc(p,mode);
    Calculate(p,mode);
  }
  else {
    bool calc(false);
    for (size_t i(0);i<p_subs->size();++i)
      if ((*p_subs)[i]->m_trig) {
	p_caller=p_proc;
	PreCalc(p,mode);
	calc=true;
     	break;
      }
    if (!calc) return m_scale[stp::fac];
    for (size_t i(0);i<p_subs->size();++i) {
      NLO_subevt *sub((*p_subs)[i]);
      if (!sub->m_trig) {
	for (size_t j(0);j<sub->m_mu2.size();++j) sub->m_mu2[j]=0.0;
	if (sub->p_ampl) {
	  sub->p_ampl->Delete();
	  sub->p_ampl=NULL;
	}
	continue;
      }
      m_p.resize(sub->m_n);
      for (size_t j(0);j<m_p.size();++j)
	m_p[j]=j<p_proc->NIn()?-sub->p_mom[j]:sub->p_mom[j];
      p_caller=static_cast<Process_Base*>(sub->p_proc);
      Calculate(Vec4D_Vector(m_p),mode);
      size_t ssz(Min(sub->m_mu2.size(),m_scale.size()));
      for (size_t j(0);j<ssz;++j) sub->m_mu2[j]=m_scale[j];
      if (sub->p_ampl) {
	sub->p_ampl->Delete();
	sub->p_ampl=NULL;
      }
      if (m_ampls.size()) {
	sub->p_ampl=m_ampls.back();
	for (Cluster_Amplitude *ampl(sub->p_ampl);
	     ampl;ampl=ampl->Next()) ampl->SetProc(sub->p_proc);
	m_ampls.pop_back();
      }
    }
  }
  if (p_proc && p_proc->Integrator()->ISR()) p_cpls->Calculate();
  return m_scale[stp::fac];
}
