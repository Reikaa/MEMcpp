#include <iostream>
#include <vector>

#include "TLorentzVector.h"
#include "TMath.h"

#include "jacobianD.h"

double computeJacobianD(std::vector<TLorentzVector> p, double sqrt_s){
	
	double E1  = p.at(0).E();
	double p1x = p.at(0).Px();
	double p1y = p.at(0).Py();
	double p1z = p.at(0).Pz();

	double E2  = p.at(1).E();
	double p2x = p.at(1).Px();
	double p2y = p.at(1).Py();
	double p2z = p.at(1).Pz();

	double E3  = p.at(2).E();
	double p3x = p.at(2).Px();
	double p3y = p.at(2).Py();
	double p3z = p.at(2).Pz();

	double E4  = p.at(3).E();
	double p4x = p.at(3).Px();
	double p4y = p.at(3).Py();
	double p4z = p.at(3).Pz();

	double E5  = p.at(4).E();
	double p5x = p.at(4).Px();
	double p5y = p.at(4).Py();
	double p5z = p.at(4).Pz();

	double E6  = p.at(5).E();
	double p6x = p.at(5).Px();
	double p6y = p.at(5).Py();
	double p6z = p.at(5).Pz();

	double E34  = E3 + E4;
	double p34x = p3x + p4x;
	double p34y = p3y + p4y;
	double p34z = p3z + p4z;

	double E56  = E5 + E6;
	double p56x = p5x + p6x;
	double p56y = p5y + p6y;
	double p56z = p5z + p6z;

    double jac = E3*(E5*
             (p34z*(p1y*p2z*p56x - p1x*p2z*p56y - p1y*p2x*p56z + 
                  p1x*p2y*p56z) + 
               p1z*(-(p2z*p34y*p56x) + p2z*p34x*p56y - 
                  p2y*p34x*p56z + p2x*p34y*p56z)) + 
            (E56*p2z - E2*p56z)*
             (p1z*p34y*p5x - p1y*p34z*p5x - p1z*p34x*p5y + 
               p1x*p34z*p5y) + 
            (E56*(p1z*p2y*p34x - p1z*p2x*p34y + p1y*p2x*p34z - 
                  p1x*p2y*p34z) + 
               E2*(p1z*p34y*p56x - p1y*p34z*p56x - p1z*p34x*p56y + 
                  p1x*p34z*p56y))*p5z) + 
         E34*(E5*p2z*(p1z*p3y*p56x - p1y*p3z*p56x - p1z*p3x*p56y + 
               p1x*p3z*p56y) + 
            E5*(p1z*p2y*p3x - p1z*p2x*p3y + p1y*p2x*p3z - 
               p1x*p2y*p3z)*p56z - 
            (E56*p2z - E2*p56z)*
             (p1z*p3y*p5x - p1y*p3z*p5x - p1z*p3x*p5y + p1x*p3z*p5y)
              - (E56*(p1z*p2y*p3x - p1z*p2x*p3y + p1y*p2x*p3z - 
                  p1x*p2y*p3z) + 
               E2*(p1z*p3y*p56x - p1y*p3z*p56x - p1z*p3x*p56y + 
                  p1x*p3z*p56y))*p5z) + 
         E1*(E5*(p2z*(-(p34z*p3y*p56x) + p34y*p3z*p56x + 
                  p34z*p3x*p56y - p34x*p3z*p56y) + 
               (-(p2y*p34z*p3x) + p2x*p34z*p3y + p2y*p34x*p3z - 
                  p2x*p34y*p3z)*p56z) + 
            (E56*p2z - E2*p56z)*
             (p34z*p3y*p5x - p34y*p3z*p5x - p34z*p3x*p5y + 
               p34x*p3z*p5y) + 
            (E56*(p2y*p34z*p3x - p2x*p34z*p3y - p2y*p34x*p3z + 
                  p2x*p34y*p3z) + 
               E2*(p34z*p3y*p56x - p34y*p3z*p56x - p34z*p3x*p56y + 
                  p34x*p3z*p56y))*p5z);

	jac /= 8.*16.*pow(TMath::Pi()*sqrt_s,2.);

	//std::cout << "jac=" << abs(jac) << std::endl;
	
	if(abs(jac) < JAC_MIN)
		return -1.;
	else
		return 1./TMath::Abs(jac);
}
