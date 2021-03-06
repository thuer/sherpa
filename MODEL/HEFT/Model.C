#include "MODEL/Main/Model_Base.H"

namespace MODEL {

  class HEFT: public Model_Base {
  private:

    int  m_ckmorder;

    void FixEWParameters();
    void FixCKM();
    void FixEFT();

    void ParticleInit();

    void InitQEDVertices();
    void InitQCDVertices();
    void InitEWVertices();
    void InitEFTVertices();

  public :

    HEFT(std::string,std::string);
    bool ModelInit(const PDF::ISR_Handler_Map& isr);
    void InitVertices();

  };

}

#include "MODEL/Main/Running_AlphaQED.H"
#include "MODEL/Main/Running_AlphaS.H"
#include "MODEL/Main/Strong_Coupling.H"
#include "MODEL/Main/Running_Fermion_Mass.H"
#include "MODEL/HEFT/Effective_Higgs_Coupling.H"
#include "MODEL/Main/Single_Vertex.H"
#include "PDF/Main/ISR_Handler.H"
#include "ATOOLS/Org/Message.H"
#include "ATOOLS/Org/Data_Reader.H"
#include "ATOOLS/Org/Exception.H"
#include "ATOOLS/Org/MyStrStream.H"

using namespace MODEL;
using namespace ATOOLS;
using namespace std;

DECLARE_GETTER(HEFT,"HEFT",Model_Base,Model_Arguments);

Model_Base *Getter<Model_Base,Model_Arguments,HEFT>::
operator()(const Model_Arguments &args) const
{
  return new HEFT(args.m_path,args.m_file);
}

void Getter<Model_Base,Model_Arguments,HEFT>::
PrintInfo(ostream &str,const size_t width) const
{ 
  str<<"The Standard Model\n";
  str<<setw(width+4)<<" "<<"{\n"
     <<setw(width+7)<<" "<<"parameter specification [keyword=value]\n"
     <<setw(width+7)<<" "<<"- EW_SCHEME (values 0,1,3, EW input schemes, see documentation)\n"
     <<setw(width+7)<<" "<<"- WIDTH_SCHEME (Fixed or CMS, see documentation)\n"
     <<setw(width+7)<<" "<<"- ALPHAS(MZ) (strong coupling at MZ)\n"
     <<setw(width+7)<<" "<<"- ORDER_ALPHAS (0,1,2 -> 1, 2, 3-loop running)\n"
     <<setw(width+7)<<" "<<"- 1/ALPHAQED(0) (alpha QED Thompson limit)\n"
     <<setw(width+7)<<" "<<"- ALPHAQED_DEFAULT_SCALE (scale for alpha_QED default)\n"
     <<setw(width+7)<<" "<<"- SIN2THETAW (weak mixing angle)\n"
     <<setw(width+7)<<" "<<"- VEV (Higgs vev)\n"
     <<setw(width+7)<<" "<<"- CKMORDER (0,1,2,3 - order of CKM expansion in Cabibbo angle)\n"
     <<setw(width+7)<<" "<<"- CABIBBO (Cabibbo angle in Wolfenstein parameterization)\n"
     <<setw(width+7)<<" "<<"- A (Wolfenstein A)\n"
     <<setw(width+7)<<" "<<"- RHO (Wolfenstein Rho)\n"
     <<setw(width+7)<<" "<<"- ETA (Wolfenstein Eta)\n"
     <<setw(width+4)<<" "<<"}";
  str<<"Infrared continuation of alphaS:\n";
  str<<setw(width+4)<<" "<<"{\n"
     <<setw(width+7)<<" "<<"- AS_FORM (values 0,1,2,3,10, see documentation)\n"
     <<setw(width+7)<<" "<<"- Q2_AS (corresponding infrared parameter, see documentation)\n"
     <<setw(width+4)<<" "<<"}";
}

HEFT::HEFT(string _dir,string _file) :
  Model_Base(_dir,_file,1)
{
  m_name="HEFT";
  ParticleInit();
  CustomContainerInit();
}

void HEFT::ParticleInit()
{
  s_kftable[kf_none] = new ATOOLS::Particle_Info(kf_none,-1,0,0,0,0,-1,0,1,0,"no_particle","no_particle","no_particle", "no_particle", 1,1);
  //add SM particles
  //kf_code,mass,width,charge,strong,spin,majorana,take,stable,massive,idname,antiname,texname,antitexname
  s_kftable[kf_d]      = new Particle_Info(kf_d,0.01,.0,-1,3,1,0,1,1,0,"d","db", "d", "\\bar{d}");
  s_kftable[kf_u]      = new Particle_Info(kf_u,0.005,.0,2,3,1,0,1,1,0,"u","ub", "u", "\\bar{u}");
  s_kftable[kf_s]      = new Particle_Info(kf_s,0.2,.0,-1,3,1,0,1,1,0,"s","sb", "s", "\\bar{s}");
  s_kftable[kf_c]      = new Particle_Info(kf_c,1.42,.0,2,3,1,0,1,1,0,"c","cb", "c", "\\bar{c}");
  s_kftable[kf_b]      = new Particle_Info(kf_b,4.8,.0,-1,3,1,0,1,1,0,"b","bb", "b", "\\bar{b}");
  s_kftable[kf_t]      = new Particle_Info(kf_t,173.21,2.0,2,3,1,0,1,0,1,"t","tb", "t", "\\bar{t}");
  s_kftable[kf_e]      = new Particle_Info(kf_e,0.000511,.0,-3,0,1,0,1,1,0,"e-","e+", "e^{-}", "e^{+}");
  s_kftable[kf_nue]    = new Particle_Info(kf_nue,.0,.0,0,0,1,0,1,1,0,"ve","veb", "\\nu_{e}", "\\bar{\\nu}_{e}");
  s_kftable[kf_mu]     = new Particle_Info(kf_mu,.105,.0,-3,0,1,0,1,1,0,"mu-","mu+", "\\mu^{-}", "\\mu^{+}");
  s_kftable[kf_numu]   = new Particle_Info(kf_numu,.0,.0,0,0,1,0,1,1,0,"vmu","vmub", "\\nu_{\\mu}", "\\bar{\\nu}_{\\mu}");
  s_kftable[kf_tau]    = new Particle_Info(kf_tau,1.777,2.26735e-12,-3,0,1,0,1,0,0,"tau-","tau+", "\\tau^{-}", "\\tau^{+}");
  s_kftable[kf_nutau]  = new Particle_Info(kf_nutau,.0,.0,0,0,1,0,1,1,0,"vtau","vtaub", "\\nu_{\\tau}", "\\bar{\\nu}_{\\tau}");
  s_kftable[kf_gluon]  = new Particle_Info(kf_gluon,.0,.0,0,8,2,-1,1,1,0,"G","G", "G", "G");
  s_kftable[kf_photon] = new Particle_Info(kf_photon,.0,.0,0,0,2,-1,1,1,0,"P","P","\\gamma","\\gamma");
  s_kftable[kf_Z]      = new Particle_Info(kf_Z,91.1876,2.4952,0,0,2,-1,1,0,1,"Z","Z","Z","Z");
  s_kftable[kf_Wplus]  = new Particle_Info(kf_Wplus,80.385,2.085,3,0,2,0,1,0,1,"W+","W-","W^{+}","W^{-}");
  s_kftable[kf_h0]     = new Particle_Info(kf_h0,125.,0.00407,0,0,0,-1,1,0,1,"h0","h0","h_{0}","h_{0}");
  s_kftable[kf_shgluon] = new Particle_Info(kf_shgluon,0.0,0.0,0,8,2,-1,1,1,0,"GS","GS","G_{S}","G_{S}",1);
  s_kftable[kf_gluon_qgc] = new Particle_Info(kf_gluon_qgc,0.0,0.0,0,8,4,-1,1,1,0,"G4","G4","G_{4}","G_{4}",1);
  ReadParticleData();
  AddStandardContainers();
}

bool HEFT::ModelInit(const PDF::ISR_Handler_Map& isr)
{
  p_dataread->RereadInFile();
  FixEWParameters();  
  FixCKM();
  FixEFT();
  SetAlphaQCD(isr);
  SetRunningFermionMasses();
  ATOOLS::OutputParticles(msg->Info());
  ATOOLS::OutputContainers(msg->Info());
  for (MODEL::ScalarNumbersMap::iterator it=p_numbers->begin();
       it!=p_numbers->end();++it) DEBUG_INFO(it->first+" = "<<it->second);
  for (MODEL::ScalarConstantsMap::iterator it=p_constants->begin();
       it!=p_constants->end();++it) DEBUG_INFO(it->first+" = "<<it->second);
  for (MODEL::ComplexConstantsMap::iterator it=p_complexconstants->begin();
       it!=p_complexconstants->end();++it) DEBUG_INFO(it->first+" = "<<it->second);
  return true;
}

void HEFT::FixEWParameters()
{
  double alphaQED0;
  Complex csin2thetaW, ccos2thetaW, cvev, I(0.,1.);
  string yukscheme=p_dataread->GetValue<string>("YUKAWA_MASSES","Running");
  p_numbers->insert(make_pair(string("YukawaScheme"), yukscheme=="Running"));
  string widthscheme=p_dataread->GetValue<string>("WIDTH_SCHEME","CMS");
  p_numbers->insert(make_pair(string("WidthScheme"), widthscheme=="CMS"));
  int ewscheme=p_dataread->GetValue<int>("EW_SCHEME",1);
  double MW=Flavour(kf_Wplus).Mass(), GW=Flavour(kf_Wplus).Width();
  double MZ=Flavour(kf_Z).Mass(), GZ=Flavour(kf_Z).Width();
  double MH=Flavour(kf_h0).Mass(), GH=Flavour(kf_h0).Width();
  switch (ewscheme) {
  case 0:
    // all SM parameters given explicitly
    alphaQED0=1./p_dataread->GetValue<double>("1/ALPHAQED(0)",137.03599976);
    aqed=new Running_AlphaQED(alphaQED0);
    aqed->SetDefault(alphaQED0=(*aqed)(p_dataread->GetValue<double>
				       ("ALPHAQED_DEFAULT_SCALE",sqr(MZ))));
    csin2thetaW=p_dataread->GetValue<double>("SIN2THETAW",0.23);
    ccos2thetaW=1.-csin2thetaW;
    cvev=p_dataread->GetValue<double>("VEV",246.);
    if (widthscheme=="CMS") THROW(not_implemented,"No CMS in EW_SCHEME=0");
    break;
  case 1: {
    // SM parameters given by alphaQED0, M_W, M_Z, M_H
    alphaQED0=1./p_dataread->GetValue<double>("1/ALPHAQED(0)",137.03599976);
    aqed=new Running_AlphaQED(alphaQED0);
    aqed->SetDefault(alphaQED0=(*aqed)(p_dataread->GetValue<double>
				       ("ALPHAQED_DEFAULT_SCALE",sqr(MZ))));
    ccos2thetaW=sqr(MW/MZ);
    csin2thetaW=1.-ccos2thetaW;
    cvev=2.*MW*sqrt(csin2thetaW/(4.*M_PI*aqed->Default()));
    if (widthscheme=="CMS") {
      Complex muW2(MW*(MW-I*GW)), muZ2(MZ*(MZ-I*GZ)), muH2(MH*(MH-I*GH));
      ccos2thetaW=muW2/muZ2;
      csin2thetaW=1.-ccos2thetaW;
      cvev=2.*sqrt(muW2*csin2thetaW/(4.*M_PI*aqed->Default()));
    }
    break;
  }
  case 3: {
    //gmu scheme
    double GF=p_dataread->GetValue<double>("GF",1.16639e-5);
    csin2thetaW=1.-sqr(MW/MZ);
    ccos2thetaW=1.-csin2thetaW;
    cvev=1./(pow(2.,0.25)*sqrt(GF));
    alphaQED0=1./p_dataread->GetValue<double>("1/ALPHAQED(0)",137.03599976);
    aqed=new Running_AlphaQED(alphaQED0);
    aqed->SetDefault(sqrt(2.)*GF/M_PI*sqr(MW)*std::abs(csin2thetaW));
    alphaQED0=aqed->Default();
    if (widthscheme=="CMS") {
      Complex muW2(MW*(MW-I*GW)), muZ2(MZ*(MZ-I*GZ)), muH2(MH*(MH-I*GH));
      ccos2thetaW=muW2/muZ2;
      csin2thetaW=1.-ccos2thetaW;
      cvev=1./(pow(2.,0.25)*sqrt(GF));
      break;
    }
    break;
  }
  case 4: {
    // FeynRules scheme, inputs: alphaQED, GF, M_Z, M_H
    double GF=p_dataread->GetValue<double>("GF",1.16639e-5);
    alphaQED0=1./p_dataread->GetValue<double>("1/ALPHAQED(0)",137.03599976);
    aqed=new Running_AlphaQED(alphaQED0);
    aqed->SetDefault(alphaQED0);
    MW = sqrt(sqr(MZ)/2. + sqrt(pow(MZ,4)/4. - (alphaQED0*M_PI*sqr(MZ))/(GF*sqrt(2.))));
    Flavour(kf_Wplus).SetMass(MW);
    
    csin2thetaW=1.-sqr(MW/MZ);
    ccos2thetaW=1.-csin2thetaW;
    cvev=1./(pow(2.,0.25)*sqrt(GF));

    aqed=new Running_AlphaQED(alphaQED0);
    if (widthscheme=="CMS") {
      Complex muW2(MW*(MW-I*GW)), muZ2(MZ*(MZ-I*GZ)), muH2(MH*(MH-I*GH));
      ccos2thetaW=muW2/muZ2;
      csin2thetaW=1.-ccos2thetaW;
      cvev=1./(pow(2.,0.25)*sqrt(GF));
      break;
    }
    break;
  }
  default:
    THROW(not_implemented, "Unknown EW_SCHEME="+ToString(ewscheme));
    break;
  }
  p_functions->insert(make_pair(string("alpha_QED"),aqed));
  p_constants->insert(make_pair(string("alpha_QED"),alphaQED0));
  p_complexconstants->insert(make_pair(string("ccos2_thetaW"),ccos2thetaW));
  p_complexconstants->insert(make_pair(string("csin2_thetaW"),csin2thetaW));
  p_complexconstants->insert(make_pair(string("cvev"), cvev));
}

void HEFT::FixCKM()
{
  CMatrix CKM(3);
  for (int i=0;i<3;i++) {
    for (int j=i;j<3;j++) CKM[i][j] = CKM[j][i] = Complex(0.,0.);
    CKM[i][i] = Complex(1.,0.);
  }
  double Cabibbo=0.0,A=.8,rho,eta;
  m_ckmorder     = p_dataread->GetValue<int>("CKMORDER",0);  
  if (m_ckmorder>0) {
    Cabibbo    = p_dataread->GetValue<double>("CABIBBO",0.2272);
    CKM[0][0] += sqr(Cabibbo)/2. * Complex(-1.,0.);
    CKM[1][1] += sqr(Cabibbo)/2. * Complex(-1.,0.);
    CKM[0][1] += Cabibbo * Complex( 1.,0.);
    CKM[1][0] += Cabibbo * Complex(-1.,0.);
  }
  if (m_ckmorder>1) {
    A          = p_dataread->GetValue<double>("A",0.818);
    CKM[1][2] += A*sqr(Cabibbo)  * Complex( 1.,0.);
    CKM[2][1] += A*sqr(Cabibbo)  * Complex(-1.,0.);
  }
  if (m_ckmorder>2) {
    eta        = p_dataread->GetValue<double>("ETA",0.349);
    rho        = p_dataread->GetValue<double>("RHO",0.227);
    CKM[0][2] += A*pow(Cabibbo,3) * Complex(rho,-eta);
    CKM[2][0] += A*pow(Cabibbo,3) * Complex(1.-rho,-eta);
  }
  for (size_t i(0);i<3;++i)
    for (size_t j(0);j<3;++j)
      p_complexconstants->insert
	(make_pair("CKM_"+ToString(i)+"_"+ToString(j),CKM[i][j]));
  for (size_t i(0);i<3;++i)
    for (size_t j(0);j<3;++j)
      p_complexconstants->insert
	(make_pair("L_CKM_"+ToString(i)+"_"+ToString(j),i==j?1.0:0.0));
}

void HEFT::FixEFT()
{
  Complex eh(2./3.,0.);
  double ph(-2.*47./18.); double pfac(1.);
  if (p_dataread->GetValue<int>("FINITE_TOP_MASS",0)==1) {
    double hm=Flavour(kf_h0).Mass();
    double tm=Flavour(kf_t).Mass();
    Effective_Higgs_Coupling ehc(hm);
    eh = ehc.GetFermionContribution(tm);
    double taut(sqr(hm/2./tm));
    pfac += -56./705.*taut - 32./987.*sqr(taut);
  }
  if (p_dataread->GetValue<int>("FINITE_W_MASS",0)==1) {
    double hm=Flavour(kf_h0).Mass();
    double Wm=Flavour(kf_Wplus).Mass();
    double tauW(sqr(hm/2./Wm));
    pfac += 66./235.*tauW + 228./1645.*sqr(tauW) + 696./8225.*tauW*sqr(tauW)
            + 5248./90475.*sqr(sqr(tauW)) + 1280./29939.*tauW*sqr(sqr(tauW))
            + 54528./1646645.*sqr(tauW*sqr(tauW));
  }
  if (p_dataread->GetValue<int>("DEACTIVATE_GGH",0)==1) eh=Complex(0.,0.);
  if (p_dataread->GetValue<int>("DEACTIVATE_PPH",0)==1) ph=0.;
  p_constants->insert(std::make_pair(std::string("h0_pp_fac"),ph*pfac));
  p_constants->insert(std::make_pair(std::string("h0_gg_fac"),real(eh)));
}

void HEFT::InitVertices()
{
  InitQEDVertices();
  InitQCDVertices();
  InitEWVertices();
  InitEFTVertices();
}

void HEFT::InitQEDVertices()
{
  if (!Flavour(kf_photon).IsOn()) return;
  Kabbala g1("g_1",sqrt(4.*M_PI*ScalarConstant("alpha_QED")));
  Kabbala cpl=g1*Kabbala("i",Complex(0.,1.));
  for (short int i=1;i<17;++i) {
    if (i==7) i=11;
    Flavour flav((kf_code)i);
    if (flav.IsOn() && flav.Charge()) {
      Kabbala Q("Q_{"+flav.TexName()+"}",flav.Charge());
      m_v.push_back(Single_Vertex());
      m_v.back().AddParticle(flav.Bar());
      m_v.back().AddParticle(flav);
      m_v.back().AddParticle(Flavour(kf_photon));
      m_v.back().Color.push_back
	(i>6?Color_Function(cf::None):
	 Color_Function(cf::D,1,2));
      m_v.back().Lorentz.push_back("FFV");
      m_v.back().cpl.push_back(cpl*Q);
      m_v.back().order[1]=1;
    } 
  }
}

void HEFT::InitQCDVertices()
{
  if (!Flavour(kf_gluon).IsOn()) return;
  Kabbala g3("g_3",sqrt(4.*M_PI*ScalarConstant("alpha_S")));
  Kabbala cpl0=g3*Kabbala("i",Complex(0.,1.));
  for (short int i=1;i<=6;++i) {
    Flavour flav((kf_code)i);
    if (!flav.IsOn()) continue; 
    m_v.push_back(Single_Vertex());
    m_v.back().AddParticle(flav.Bar());
    m_v.back().AddParticle(flav);
    m_v.back().AddParticle(Flavour(kf_gluon));
    m_v.back().Color.push_back(Color_Function(cf::T,3,2,1));
    m_v.back().Lorentz.push_back("FFV");
    m_v.back().cpl.push_back(cpl0);
    m_v.back().order[0]=1;
  }
  Kabbala cpl1=-g3;
  m_v.push_back(Single_Vertex());
  for (size_t i(0);i<3;++i) m_v.back().AddParticle(Flavour(kf_gluon));
  m_v.back().Color.push_back(Color_Function(cf::F,1,2,3));
  m_v.back().Lorentz.push_back("VVV");
  m_v.back().cpl.push_back(cpl1);
  m_v.back().order[0]=1;
  m_v.push_back(Single_Vertex());
  for (size_t i(0);i<2;++i) m_v.back().AddParticle(Flavour(kf_gluon));
  m_v.back().AddParticle(Flavour(kf_gluon_qgc));
  m_v.back().Color.push_back(Color_Function(cf::F,1,2,3));
  m_v.back().Lorentz.push_back("VVP");
  m_v.back().cpl.push_back(cpl1);
  m_v.back().order[0]=1;
  m_v.back().dec=1;
  Kabbala cpl2=g3*g3*Kabbala("i",Complex(0.,1.)); 
  m_v.push_back(Single_Vertex());
  for (size_t i(0);i<4;++i) m_v.back().AddParticle(Flavour(kf_gluon));
  for (size_t i(0);i<3;++i) m_v.back().cpl.push_back(cpl2);
  m_v.back().Color.push_back
    (Color_Function(cf::F,-1,1,2,new Color_Function(cf::F,3,4,-1)));
  m_v.back().Color.push_back
    (Color_Function(cf::F,-1,1,3,new Color_Function(cf::F,2,4,-1)));
  m_v.back().Color.push_back
    (Color_Function(cf::F,-1,1,4,new Color_Function(cf::F,2,3,-1)));
  m_v.back().Lorentz.push_back("VVVVA");
  m_v.back().Lorentz.push_back("VVVVB");
  m_v.back().Lorentz.push_back("VVVVC");
  m_v.back().order[0]=2;
  m_v.back().dec=-1;
}

void HEFT::InitEWVertices()
{
  Kabbala two(Kabbala("2",2.0)), three(Kabbala("3",3.0));
  Kabbala I("i",Complex(0.,1.)), rt2("\\sqrt(2)",sqrt(2.0));
  Kabbala g1("g_1",sqrt(4.*M_PI*ScalarConstant("alpha_QED")));
  Kabbala sintW("\\sin\\theta_W",sqrt(ComplexConstant("csin2_thetaW")));
  Kabbala costW("\\cos\\theta_W",sqrt(ComplexConstant("ccos2_thetaW")));
  Kabbala g2(g1/sintW), vev("v_{EW}",ComplexConstant("cvev"));
  if (Flavour(kf_Wplus).IsOn()) {
    Kabbala cpl=I/rt2*g2;
    for (short int i=1;i<17;i+=2) {
      if (i==7) i=11;
      Flavour flav1((kf_code)i);
      if (!flav1.IsOn()) continue;
      for (short int j=2;j<18;j+=2) {
	if (j==8) j=12;
	if ((i<10 && j>10) || (i>10 && j<10)) continue;
	Flavour flav2((kf_code)j);
	if (!flav2.IsOn()) continue;
	std::string ckmstr=(i<10?"CKM_":"L_CKM_")+
	  ToString(((i%10)-1)/2)+"_"+ToString((j%10)/2-1);
	Kabbala ckm(ckmstr,ComplexConstant(ckmstr));
	if (std::abs(ckm.Value())==0.0) continue;
	m_v.push_back(Single_Vertex());
	m_v.back().AddParticle(flav1.Bar());
	m_v.back().AddParticle(flav2);
	m_v.back().AddParticle(Flavour(kf_Wplus).Bar());
	m_v.back().Color.push_back
	  (i>6?Color_Function(cf::None):
	   Color_Function(cf::D,1,2));
	m_v.back().Lorentz.push_back("FFVL");
	m_v.back().cpl.push_back(cpl*ckm);
	m_v.back().order[1]=1;
	m_v.push_back(Single_Vertex());
	m_v.back().AddParticle(flav2.Bar());
	m_v.back().AddParticle(flav1);
	m_v.back().AddParticle(Flavour(kf_Wplus));
	m_v.back().Color.push_back
	  (i>6?Color_Function(cf::None):
	   Color_Function(cf::D,1,2));
	m_v.back().Lorentz.push_back("FFVL");
	m_v.back().cpl.push_back(cpl*ckm);
	m_v.back().order[1]=1;
      } 
    }
  }
  if (Flavour(kf_Z).IsOn()) {
    for (short int i=1;i<17;++i) {
      if (i==7) i=11;
      Flavour flav((kf_code)i);
      if (!flav.IsOn()) continue;
      Kabbala Q("Q_{"+flav.TexName()+"}",flav.Charge());
      Kabbala W("T_{"+flav.TexName()+"}",flav.IsoWeak());
      m_v.push_back(Single_Vertex());
      m_v.back().AddParticle(flav.Bar());
      m_v.back().AddParticle(flav);
      m_v.back().AddParticle(Flavour(kf_Z));
      m_v.back().Color.push_back
	(i>6?Color_Function(cf::None):
	 Color_Function(cf::D,1,2));
      m_v.back().Color.push_back
	(i>6?Color_Function(cf::None):
	 Color_Function(cf::D,1,2));
      m_v.back().Lorentz.push_back("FFVL");
      m_v.back().Lorentz.push_back("FFVR");
      m_v.back().cpl.push_back(I/costW*(-Q*sintW+W/sintW)*g1);
      m_v.back().cpl.push_back(-I/costW*Q*sintW*g1);
      m_v.back().order[1]=1;
    } 
  }
  if (Flavour(kf_h0).IsOn()) {
    Kabbala cpl(-I/vev);
    for (short int i=1;i<17;++i) {
      if (i==7) i=11;
      Flavour flav((kf_code)i);
      if (!flav.IsOn() || flav.Yuk()==0.0) continue;
      double m=(ScalarNumber("YukawaScheme")==0)?flav.Yuk():
	ScalarFunction("m"+flav.IDName(),sqr(Flavour(kf_h0).Mass(true)));
      Kabbala M;
      if (ScalarNumber("WidthScheme")!=0)
        M=Kabbala("M_{"+flav.TexName()+"}(m_h^2)",
		  sqrt(m*m-Complex(0.0,m*flav.Width())));
      else M=Kabbala("M_{"+flav.TexName()+"}(m_h^2)",m);
      m_v.push_back(Single_Vertex());
      m_v.back().AddParticle(flav.Bar());
      m_v.back().AddParticle(flav);
      m_v.back().AddParticle(Flavour(kf_h0));
      m_v.back().Color.push_back
	(i>6?Color_Function(cf::None):
	 Color_Function(cf::D,1,2));
      m_v.back().Lorentz.push_back("FFS");
      m_v.back().cpl.push_back(cpl*M);
      m_v.back().order.push_back(1);
    } 
  }
  if (Flavour(kf_Wplus).IsOn()) {
    if (Flavour(kf_photon).IsOn()) {
      m_v.push_back(Single_Vertex());
      m_v.back().AddParticle(Flavour(kf_Wplus).Bar());
      m_v.back().AddParticle(Flavour(kf_Wplus));
      m_v.back().AddParticle(Flavour(kf_photon));
      m_v.back().Color.push_back(Color_Function(cf::None));
      m_v.back().Lorentz.push_back("VVV");
      m_v.back().cpl.push_back(I*g1);
      m_v.back().order[1]=1;
    }      
    if (Flavour(kf_Z).IsOn()) {
      m_v.push_back(Single_Vertex());
      m_v.back().AddParticle(Flavour(kf_Wplus).Bar());
      m_v.back().AddParticle(Flavour(kf_Wplus));
      m_v.back().AddParticle(Flavour(kf_Z));
      m_v.back().Color.push_back(Color_Function(cf::None));
      m_v.back().Lorentz.push_back("VVV");
      m_v.back().cpl.push_back(I*g2*costW);
      m_v.back().order[1]=1;
    }
    m_v.push_back(Single_Vertex());
    m_v.back().AddParticle(Flavour(kf_Wplus).Bar());
    m_v.back().AddParticle(Flavour(kf_Wplus).Bar());
    m_v.back().AddParticle(Flavour(kf_Wplus));
    m_v.back().AddParticle(Flavour(kf_Wplus));
    m_v.back().cpl.push_back(-I*g2*g2);
    m_v.back().Color.push_back(Color_Function(cf::None));
    m_v.back().Lorentz.push_back("VVVV");
    m_v.back().order[1]=2;
    m_v.push_back(Single_Vertex());
    m_v.back().AddParticle(Flavour(kf_Wplus).Bar());
    m_v.back().AddParticle(Flavour(kf_Wplus));
    m_v.back().AddParticle(Flavour(kf_photon));
    m_v.back().AddParticle(Flavour(kf_photon));
    m_v.back().cpl.push_back(I*g1*g1);
    m_v.back().Color.push_back(Color_Function(cf::None));
    m_v.back().Lorentz.push_back("VVVV");
    m_v.back().order[1]=2;
    m_v.push_back(Single_Vertex());
    m_v.back().AddParticle(Flavour(kf_Wplus).Bar());
    m_v.back().AddParticle(Flavour(kf_Wplus));
    m_v.back().AddParticle(Flavour(kf_photon));
    m_v.back().AddParticle(Flavour(kf_Z));
    m_v.back().cpl.push_back(I*g1*g2*costW);
    m_v.back().Color.push_back(Color_Function(cf::None));
    m_v.back().Lorentz.push_back("VVVV");
    m_v.back().order[1]=2;
    m_v.push_back(Single_Vertex());
    m_v.back().AddParticle(Flavour(kf_Wplus).Bar());
    m_v.back().AddParticle(Flavour(kf_Wplus));
    m_v.back().AddParticle(Flavour(kf_Z));
    m_v.back().AddParticle(Flavour(kf_Z));
    m_v.back().cpl.push_back(I*g2*g2*costW*costW);
    m_v.back().Color.push_back(Color_Function(cf::None));
    m_v.back().Lorentz.push_back("VVVV");
    m_v.back().order[1]=2;
  }
  if (Flavour(kf_h0).IsOn()) {
    if (Flavour(kf_Wplus).IsOn()) {
      Kabbala M("M_W",Flavour(kf_Wplus).Mass()), cpl;
      if (ScalarNumber("WidthScheme")!=0) {
	Kabbala G("\\Gamma_W",Flavour(kf_Wplus).Width());
	M=Kabbala("M_W",sqrt((M*M-I*G*M).Value()));
      }
      m_v.push_back(Single_Vertex());
      m_v.back().AddParticle(Flavour(kf_Wplus).Bar());
      m_v.back().AddParticle(Flavour(kf_Wplus));
      m_v.back().AddParticle(Flavour(kf_h0));
      m_v.back().Color.push_back(Color_Function(cf::None));
      m_v.back().Lorentz.push_back("VVS");
      m_v.back().cpl.push_back(I*g2*M);
      m_v.back().order[1]=1;
      m_v.push_back(Single_Vertex());
      m_v.back().AddParticle(Flavour(kf_Wplus).Bar());
      m_v.back().AddParticle(Flavour(kf_Wplus));
      m_v.back().AddParticle(Flavour(kf_h0));
      m_v.back().AddParticle(Flavour(kf_h0));
      m_v.back().cpl.push_back(I*g2*g2/two);
      m_v.back().Color.push_back(Color_Function(cf::None));
      m_v.back().Lorentz.push_back("VVSS");
      m_v.back().order[1]=2;
    }
    if (Flavour(kf_Z).IsOn()) {
      Kabbala M("M_Z",Flavour(kf_Z).Mass()), cpl;
      if (ScalarNumber("WidthScheme")!=0) {
	Kabbala G("\\Gamma_Z",Flavour(kf_Z).Width());
	M=Kabbala("M_Z",sqrt((M*M-I*G*M).Value()));
      }
      m_v.push_back(Single_Vertex());
      m_v.back().AddParticle(Flavour(kf_Z));
      m_v.back().AddParticle(Flavour(kf_Z));
      m_v.back().AddParticle(Flavour(kf_h0));
      m_v.back().Color.push_back(Color_Function(cf::None));
      m_v.back().Lorentz.push_back("VVS");
      m_v.back().cpl.push_back(I*g2*M/costW);
      m_v.back().order[1]=1;
      m_v.push_back(Single_Vertex());
      m_v.back().AddParticle(Flavour(kf_Z));
      m_v.back().AddParticle(Flavour(kf_Z));
      m_v.back().AddParticle(Flavour(kf_h0));
      m_v.back().AddParticle(Flavour(kf_h0));
      m_v.back().cpl.push_back(I*g2*g2/(costW*costW*two));
      m_v.back().Color.push_back(Color_Function(cf::None));
      m_v.back().Lorentz.push_back("VVSS");
      m_v.back().order[1]=2;
    }
    Kabbala M("M_H",Flavour(kf_h0).Mass()), cpl;
    if (ScalarNumber("WidthScheme")!=0) {
      Kabbala G("\\Gamma_H",Flavour(kf_h0).Width());
      M=Kabbala("M_H",sqrt((M*M-I*G*M).Value()));
    }
    m_v.push_back(Single_Vertex());
    m_v.back().AddParticle(Flavour(kf_h0));
    m_v.back().AddParticle(Flavour(kf_h0));
    m_v.back().AddParticle(Flavour(kf_h0));
    m_v.back().Color.push_back(Color_Function(cf::None));
    m_v.back().Lorentz.push_back("SSS");
    m_v.back().cpl.push_back(-I*M*M*three/vev);
    m_v.back().order[1]=1;
    m_v.push_back(Single_Vertex());
    m_v.back().AddParticle(Flavour(kf_h0));
    m_v.back().AddParticle(Flavour(kf_h0));
    m_v.back().AddParticle(Flavour(kf_h0));
    m_v.back().AddParticle(Flavour(kf_h0));
    m_v.back().Color.push_back(Color_Function(cf::None));
    m_v.back().Lorentz.push_back("SSSS");
    m_v.back().cpl.push_back(-I*M*M*three/(vev*vev));
    m_v.back().order[1]=2;
  }
}

void HEFT::InitEFTVertices()
{
  if (!Flavour(kf_h0).IsOn()) return;
  double as(p_dataread->GetValue<double>
	    ("ALPHAS_GGH",ScalarConstant("alpha_S")));
  double aqed(1.0/p_dataread->GetValue<double>
	      ("1/ALPHAQED_PPH",1.0/ScalarConstant("alpha_QED")));
  Kabbala ghgg("g_hgg",ScalarConstant("h0_gg_fac")*
	       as/(2.0*M_PI*ComplexConstant("cvev")));
  Kabbala ghpp("g_hpp",ScalarConstant("h0_pp_fac")*
	       aqed/(2.0*M_PI*ComplexConstant("cvev")));
  Kabbala g3("g_3",sqrt(4.*M_PI*ScalarConstant("alpha_S")));
  Kabbala I("i",Complex(0.,1.));
  msg_Info()<<METHOD<<"() {\n  ggh coupling is "
	    <<ghgg.Value()<<" [ \\alpha_s = "<<as
	    <<" ]\n  ggh coupling is "<<ghpp.Value()
            <<" [ 1/\\alpha = "<<(1.0/aqed)<<" ]\n}\n";
  m_v.push_back(Single_Vertex());
  m_v.back().AddParticle(Flavour(kf_h0));
  m_v.back().AddParticle(Flavour(kf_gluon));
  m_v.back().AddParticle(Flavour(kf_gluon));
  m_v.back().Color.push_back(Color_Function(cf::G,2,3));
  m_v.back().Lorentz.push_back("HVV");
  m_v.back().cpl.push_back(I*ghgg);
  m_v.back().order[0]=2;
  m_v.back().order.push_back(1);
  m_v.push_back(Single_Vertex());
  m_v.back().AddParticle(Flavour(kf_h0));
  m_v.back().AddParticle(Flavour(kf_photon));
  m_v.back().AddParticle(Flavour(kf_photon));
  m_v.back().Color.push_back(Color_Function(cf::None));
  m_v.back().Lorentz.push_back("HVV");
  m_v.back().cpl.push_back(I*ghpp);
  m_v.back().order[1]=2;
  m_v.back().order.push_back(1);
  m_v.push_back(Single_Vertex());
  m_v.back().AddParticle(Flavour(kf_h0));
  m_v.back().AddParticle(Flavour(kf_gluon));
  m_v.back().AddParticle(Flavour(kf_gluon));
  m_v.back().AddParticle(Flavour(kf_gluon));
  m_v.back().Color.push_back(Color_Function(cf::F,2,3,4));
  m_v.back().Lorentz.push_back("HVVV");
  m_v.back().cpl.push_back(ghgg*g3);
  m_v.back().order[0]=3;
  m_v.back().order.push_back(1);
  m_v.push_back(Single_Vertex());
  m_v.back().AddParticle(Flavour(kf_h0));
  m_v.back().AddParticle(Flavour(kf_gluon));
  m_v.back().AddParticle(Flavour(kf_gluon));
  m_v.back().AddParticle(Flavour(kf_gluon_qgc));
  m_v.back().Color.push_back(Color_Function(cf::F,2,3,4));
  m_v.back().Lorentz.push_back("HVVP");
  m_v.back().cpl.push_back(ghgg*g3);
  m_v.back().order[0]=3;
  m_v.back().order.push_back(1);
  m_v.back().dec=3;
  m_v.push_back(Single_Vertex());
  for (size_t i(0);i<4;++i) m_v.back().AddParticle(Flavour(kf_gluon));
  m_v.back().AddParticle(Flavour(kf_h0));
  for (size_t i(0);i<3;++i) m_v.back().cpl.push_back(-I*ghgg*g3*g3);
  m_v.back().Color.push_back
    (Color_Function(cf::F,-1,1,2,new Color_Function(cf::F,3,4,-1)));
  m_v.back().Color.push_back
    (Color_Function(cf::F,-1,1,3,new Color_Function(cf::F,2,4,-1)));
  m_v.back().Color.push_back
    (Color_Function(cf::F,-1,1,4,new Color_Function(cf::F,2,3,-1)));
  m_v.back().Lorentz.push_back("HVVVVA");
  m_v.back().Lorentz.push_back("HVVVVB");
  m_v.back().Lorentz.push_back("HVVVVC");
  m_v.back().order[0]=4;
  m_v.back().order.push_back(1);
  m_v.back().dec=3;
  m_v.push_back(Single_Vertex());
  m_v.back().AddParticle(Flavour(kf_h0));
  m_v.back().AddParticle(Flavour(kf_shgluon));
  m_v.back().AddParticle(Flavour(kf_gluon));
  m_v.back().Color.push_back(Color_Function(cf::G,2,3));
  m_v.back().Lorentz.push_back("C4GS");
  m_v.back().cpl.push_back(-I);
  m_v.back().dec=-1;
  Kabbala cpl2=-I*g3*g3*ghgg;
  m_v.push_back(Single_Vertex());
  for (size_t i(0);i<3;++i) m_v.back().AddParticle(Flavour(kf_gluon));
  m_v.back().AddParticle(Flavour(kf_shgluon));
  for (size_t i(0);i<3;++i) m_v.back().cpl.push_back(cpl2);
  m_v.back().Color.push_back
    (Color_Function(cf::F,-1,1,2,new Color_Function(cf::F,3,4,-1)));
  m_v.back().Color.push_back
    (Color_Function(cf::F,-1,1,3,new Color_Function(cf::F,2,4,-1)));
  m_v.back().Color.push_back
    (Color_Function(cf::F,-1,1,4,new Color_Function(cf::F,2,3,-1)));
  m_v.back().Lorentz.push_back("VVVVA");
  m_v.back().Lorentz.push_back("VVVVB");
  m_v.back().Lorentz.push_back("VVVVC");
  m_v.back().order[0]=2;
  m_v.back().order.push_back(1);
  m_v.back().dec=-1;
}
