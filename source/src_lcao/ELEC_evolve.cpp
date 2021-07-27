#include "ELEC_evolve.h"
#include "LOOP_elec.h"
#include "LCAO_diago.h"
#include "../src_pw/global.h"
#include "../src_pw/symmetry_rho.h"
#include "LCAO_evolve.h"
#include "dftu.h"

ELEC_evolve::ELEC_evolve(){};
ELEC_evolve::~ELEC_evolve(){};

int ELEC_evolve::tddft;
double ELEC_evolve::td_dr2;
double ELEC_evolve::td_dt;
double ELEC_evolve::td_force_dt;
int ELEC_evolve::td_val_elec_01;
int ELEC_evolve::td_val_elec_02;
int ELEC_evolve::td_val_elec_03;
int ELEC_evolve::td_vext;
int ELEC_evolve::td_vext_dire;
double ELEC_evolve::td_timescale;
int ELEC_evolve::td_vexttype;
int ELEC_evolve::td_vextout;
int ELEC_evolve::td_dipoleout;

// this routine only serves for TDDFT using LCAO basis set
void ELEC_evolve::evolve_psi(
	const int &istep, 
	LCAO_Hamilt &uhm, 
	complex<double> ***wfc)
{
	TITLE("ELEC_evolve","eveolve_psi");
	timer::tick("ELEC_evolve","evolve_psi");

	int start_spin = -1;
	uhm.GK.reset_spin(start_spin);
	uhm.GK.allocate_pvpR();
						
	// pool parallization in future -- mohan note 2021-02-09
	for(int ik=0; ik<GlobalC::kv.nks; ik++)
	{	
		//-----------------------------------------
		//(1) prepare data for this k point.
		// copy the local potential from array.
		//-----------------------------------------
		if(GlobalV::NSPIN==2) 
		{
			GlobalV::CURRENT_SPIN = GlobalC::kv.isk[ik];
		}
		GlobalC::wf.npw = GlobalC::kv.ngk[ik];

		for(int ir=0; ir<GlobalC::pw.nrxx; ir++)
		{
			GlobalC::pot.vr_eff1[ir] = GlobalC::pot.vr_eff( GlobalV::CURRENT_SPIN, ir);
		}
		
		//--------------------------------------------
		//(2) check if we need to calculate 
		// pvpR = < phi0 | v(spin) | phiR> for a new spin.
		//--------------------------------------------
		if(GlobalV::CURRENT_SPIN == uhm.GK.get_spin() )
		{
			//GlobalV::ofs_running << " Same spin, same vlocal integration." << endl;
		}
		else
		{
			uhm.GK.reset_spin( GlobalV::CURRENT_SPIN );

			// vlocal = Vh[rho] + Vxc[rho] + Vl(pseudo)
			uhm.GK.cal_vlocal_k(GlobalC::pot.vr_eff1,GridT);
			// added by zhengdy-soc, for non-collinear case
			// integral 4 times, is there any method to simplify?
			if(GlobalV::NSPIN==4)
			{
				for(int is=1;is<4;is++)
				{
					for(int ir=0; ir<GlobalC::pw.nrxx; ir++)
					{
						GlobalC::pot.vr_eff1[ir] = GlobalC::pot.vr_eff(is, ir);
					}
					uhm.GK.cal_vlocal_k(GlobalC::pot.vr_eff1, GridT, is);
				}
			}
		}


		if(!uhm.init_s)
    	{
    	    WARNING_QUIT("Hamilt_Linear::solve_using_cg","Need init S matrix firstly");
    	}

		//--------------------------------------------
		// (3) folding matrix, 
		// and diagonalize the H matrix (T+Vl+Vnl).
		//--------------------------------------------

		// with k points
		uhm.calculate_Hk(ik);

		// Effective potential of DFT+U is added to total Hamiltonian here; Quxin adds on 20201029
		if(INPUT.dft_plus_u)
		{		
			dftu.cal_eff_pot_mat(ik, istep);

			for(int irc=0; irc<ParaO.nloc; irc++)
			{
				LM.Hloc2[irc] += dftu.pot_eff_k.at(ik).at(irc);
			}							
		}

		// Peize Lin add at 2020.04.04
		if(restart.info_load.load_H && !restart.info_load.load_H_finish)
		{
			restart.load_disk("H", ik);
			restart.info_load.load_H_finish = true;
		}			
		if(restart.info_save.save_H)
		{
			restart.save_disk("H", ik);
		}

		bool diago = true;
		if (istep >= 1) 
		{
			diago = false;
		}

		if(diago)
		{
			timer::tick("Efficience","diago_k");
			Diago_LCAO_Matrix DLM;
			DLM.solve_complex_matrix(ik, LOWF.WFC_K[ik], LOC.wfc_dm_2d.wfc_k[ik]);
			timer::tick("Efficience","diago_k");
		}
		else
		{
			timer::tick("Efficience","evolve_k");
			Evolve_LCAO_Matrix ELM;
			ELM.evolve_complex_matrix(ik, LOWF.WFC_K[ik], wfc[ik]);
			timer::tick("Efficience","evolve_k");
		}
	} // end k
			
	// LiuXh modify 2019-07-15*/
	if(!ParaO.out_hsR)
	{
		uhm.GK.destroy_pvpR();
	}

	timer::tick("ELEC_evolve","evolve_psi");
	return;	
}


void ELEC_evolve::evolve_complex_matrix(
	const int &ik, 
	complex<double>** cc, 
	complex<double>** cc_init)const
{
	TITLE("Evolve_LCAO_Matrix","evolve_complex_matrix");
	time_t time_start = time(NULL);
	GlobalV::ofs_running << " Start Time : " << ctime(&time_start);

	if (INPUT.tddft==1)
	{
/*
#ifdef __MPI
		this->using_LAPACK_complex_2(ik, cc, cc_init);
#else
		this->using_LAPACK_complex(ik, cc, cc_init);
#endif
*/
		this->using_LAPACK_complex(ik, cc, cc_init);
	}
	else
	{
		WARNING_QUIT("ELEC_evolve","only tddft==1 cando evolve");
	}

	time_t time_end = time(NULL);
	OUT_TIME("evolve(complex)", time_start, time_end);
	
	return;
}

void ELEC_evolve::using_LAPACK_complex(const int &ik, complex<double>** c, complex<double>** c_init)const
{
	TITLE("ELEC_evolve","using_LAPACK_complex");

//	Calculate the U operator

	bool bit = false;
	//HS_Matrix::saving_HS_complex(LM.Hloc2, LM.Sloc2, bit, ParaO.out_hs);

	ComplexMatrix Htmp(GlobalV::NLOCAL,GlobalV::NLOCAL);
	ComplexMatrix Stmp(GlobalV::NLOCAL,GlobalV::NLOCAL);

	for(int i=0; i<GlobalV::NLOCAL; i++)
	{
		for(int j=0; j<GlobalV::NLOCAL; j++)
		{
			Htmp(i,j) = LM.Hloc2[i*GlobalV::NLOCAL+j];
			Stmp(i,j) = LM.Sloc2[i*GlobalV::NLOCAL+j];
		}
	}


	int INFO;

	int LWORK=3*GlobalV::NLOCAL-1; //tmp
	complex<double> * WORK = new complex<double>[LWORK];
	ZEROS(WORK, LWORK);
	int IPIV[GlobalV::NLOCAL];

	LapackConnector::zgetrf( GlobalV::NLOCAL, GlobalV::NLOCAL, Stmp, GlobalV::NLOCAL, IPIV, &INFO);
	LapackConnector::zgetri( GlobalV::NLOCAL, Stmp, GlobalV::NLOCAL, IPIV, WORK, LWORK, &INFO);


	ComplexMatrix S_plus_H(GlobalV::NLOCAL,GlobalV::NLOCAL);
	S_plus_H = Stmp*Htmp;


	ComplexMatrix Denominator(GlobalV::NLOCAL,GlobalV::NLOCAL);
	for (int i=0; i<GlobalV::NLOCAL; i++)
	{
		for (int j=0; j<GlobalV::NLOCAL; j++)
		{
			/*   real(Denominator(i,j)) = -imag(S_plus_H(i,j));
				 imag(Denominator(i,j)) = real(S_plus_H(i,j));*/
			Denominator(i,j) = std::complex<double>( -S_plus_H(i,j).imag(), S_plus_H(i,j).real() );
		}
	}

	ComplexMatrix Idmat(GlobalV::NLOCAL,GlobalV::NLOCAL);
	for(int i=0; i<GlobalV::NLOCAL; i++)
	{
		for(int j=0; j<GlobalV::NLOCAL; j++)
		{
			if(i==j) Idmat(i,j) = complex<double>(1.0, 0.0);
			else Idmat(i,j) = complex<double>(0.0, 0.0);
		}
	}
	double delta_t;
	//      delta_t = 0.2;	//identity: fs;
	ComplexMatrix Numerator(GlobalV::NLOCAL,GlobalV::NLOCAL);
	Numerator = Idmat - 0.5*INPUT.mdp.dt*41.34*Denominator;
	Denominator = Idmat + 0.5*INPUT.mdp.dt*41.34*Denominator;

	int info;
	int lwork=3*GlobalV::NLOCAL-1; //tmp
	complex<double> * work = new complex<double>[lwork];
	ZEROS(work, lwork);
	int ipiv[GlobalV::NLOCAL];

	LapackConnector::zgetrf( GlobalV::NLOCAL, GlobalV::NLOCAL, Denominator, GlobalV::NLOCAL, ipiv, &info);
	LapackConnector::zgetri( GlobalV::NLOCAL, Denominator, GlobalV::NLOCAL, ipiv, work, lwork, &info);

	ComplexMatrix U_operator(GlobalV::NLOCAL,GlobalV::NLOCAL);

	U_operator = Numerator*Denominator;

	for(int i=0; i<GlobalV::NBANDS; i++)
	{
		complex<double> ccc[GlobalV::NLOCAL];
		for(int j=0; j<GlobalV::NLOCAL; j++)
		{	
			ccc[j] = (0.0,0.0);
			for(int k=0; k<GlobalV::NLOCAL; k++)
			{
				 ccc[j] += U_operator(j,k)*c_init[i][k];
			}
		}
		for(int j=0; j<GlobalV::NLOCAL; j++)
		{
			c[i][j] = ccc[j];
			LOWF.WFC_K[ik][i][j] = ccc[j];
		}	
	}

//	delete[] work;
//	delete[] ipiv;

	return;
}

void ELEC_evolve::using_LAPACK_complex_2(
	const int &ik, 
	complex<double>** c, 
	complex<double>** c_init)const
{

	ComplexMatrix Htmp(GlobalV::NLOCAL,GlobalV::NLOCAL);
	ComplexMatrix Stmp(GlobalV::NLOCAL,GlobalV::NLOCAL);

	int ir=0;
	int ic=0;
	for (int i=0; i<GlobalV::NLOCAL; i++)
	{
		complex<double>* lineH = new complex<double>[GlobalV::NLOCAL-i];
		complex<double>* lineS = new complex<double>[GlobalV::NLOCAL-i];
		ZEROS(lineH, GlobalV::NLOCAL-i);
		ZEROS(lineS, GlobalV::NLOCAL-i);

		ir = ParaO.trace_loc_row[i];
		if (ir>=0)
		{
			// data collection
			for (int j=i; j<GlobalV::NLOCAL; j++)
			{
				ic = ParaO.trace_loc_col[j];
				if (ic>=0)
				{
					//lineH[j-i] = H[ir*ParaO.ncol+ic];
					lineH[j-i] = LM.Hloc2[ir*ParaO.ncol+ic];
					//lineS[j-i] = S[ir*ParaO.ncol+ic];
					lineS[j-i] = LM.Sloc2[ir*ParaO.ncol+ic];
				}
			}
		}
		else
		{
			//do nothing
		}

		Parallel_Reduce::reduce_complex_double_pool(lineH,GlobalV::NLOCAL-i);
		Parallel_Reduce::reduce_complex_double_pool(lineS,GlobalV::NLOCAL-i);

		if (GlobalV::DRANK==0)
		{
			for (int j=i; j<GlobalV::NLOCAL; j++)
			{
				//g1 << " " << lineH[j-i];
				Htmp(i,j) = lineH[j-i];
				//g2 << " " << lineS[j-i];
				Stmp(i,j) = lineS[j-i];
			}
		}
		delete[] lineH;
		delete[] lineS;
	}

	int INFO=0;

	int LWORK=3*GlobalV::NLOCAL-1; //tmp
	complex<double> * WORK = new complex<double>[LWORK];
	ZEROS(WORK, LWORK);
	int IPIV[GlobalV::NLOCAL];

	LapackConnector::zgetrf( GlobalV::NLOCAL, GlobalV::NLOCAL, Stmp, GlobalV::NLOCAL, IPIV, &INFO);
	LapackConnector::zgetri( GlobalV::NLOCAL, Stmp, GlobalV::NLOCAL, IPIV, WORK, LWORK, &INFO);

	ComplexMatrix S_plus_H(GlobalV::NLOCAL,GlobalV::NLOCAL);
	S_plus_H = Stmp*Htmp;

	ComplexMatrix Denominator(GlobalV::NLOCAL,GlobalV::NLOCAL);
	for (int i=0; i<GlobalV::NLOCAL; i++)
	{
		for (int j=0; j<GlobalV::NLOCAL; j++)
		{
			/*   real(Denominator(i,j)) = -imag(S_plus_H(i,j));
				 imag(Denominator(i,j)) = real(S_plus_H(i,j));*/
			Denominator(i,j) = std::complex<double>( -S_plus_H(i,j).imag(), S_plus_H(i,j).real() );
		}
	}

	ComplexMatrix Idmat(GlobalV::NLOCAL,GlobalV::NLOCAL);
	for(int i=0; i<GlobalV::NLOCAL; i++)
	{
		for(int j=0; j<GlobalV::NLOCAL; j++)
		{
			if(i==j) 
			{
				Idmat(i,j) = complex<double>(1.0, 0.0);
			}
			else 
			{
				Idmat(i,j) = complex<double>(0.0, 0.0);
			}
		}
	}

	double delta_t=0.0;

	ComplexMatrix Numerator(GlobalV::NLOCAL,GlobalV::NLOCAL);
	Numerator = Idmat - 0.5*INPUT.mdp.dt*41.34*Denominator;
	Denominator = Idmat + 0.5*INPUT.mdp.dt*41.34*Denominator;

	int info=0;
	int lwork=3*GlobalV::NLOCAL-1; //tmp
	complex<double>* work = new complex<double>[lwork];
	ZEROS(work, lwork);
	int ipiv[GlobalV::NLOCAL];

	LapackConnector::zgetrf( GlobalV::NLOCAL, GlobalV::NLOCAL, Denominator, GlobalV::NLOCAL, ipiv, &info);
	LapackConnector::zgetri( GlobalV::NLOCAL, Denominator, GlobalV::NLOCAL, ipiv, work, lwork, &info);

	ComplexMatrix U_operator(GlobalV::NLOCAL,GlobalV::NLOCAL);
	U_operator = Numerator*Denominator;

	// Calculate wave function at t+delta t

	//	cout << "wave function coe at t+delta t !" << endl;

	for(int i=0; i<GlobalV::NBANDS; i++)
	{
		complex<double> ccc[GlobalV::NLOCAL];
		for(int j=0; j<GlobalV::NLOCAL; j++)
		{	
			ccc[j] = (0.0,0.0);
			for(int k=0; k<GlobalV::NLOCAL; k++)
			{
				ccc[j] += U_operator(j,k)*c_init[i][k];
			}
		}
		for(int j=0; j<GlobalV::NLOCAL; j++)
		{
			c[i][j] = ccc[j];
			LOWF.WFC_K[ik][i][j] = ccc[j];
		}	
	}

	delete[] work; // mohan add 2021-05-26

	return;
}
