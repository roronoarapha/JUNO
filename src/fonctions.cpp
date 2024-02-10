#include "fonctions.h"
#include <math.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
using namespace std;

double mass_neutron=939.565;//MeV
double mass_proton=938.272;//MeV
double mass_positron=0.510999;//MeV

double fission_energy_U_235=202.36;//MeV
double fission_energy_U_238=205.99;//MeV
double fission_energy_PU_239=211.12;//MeV
double fission_energy_PU_241=214.26;//MeV

double fission_fraction_U_235=0.58;//MeV
double fission_fraction_U_238=0.07;//MeV
double fission_fraction_PU_239=0.30;//MeV
double fission_fraction_PU_241=0.05;//MeV

double sin2_teta_12=0.32;
double sin2_2teta_13=0.1;   

double teta_12=asin(sqrt(sin2_teta_12));
double teta_13=asin(sqrt(sin2_2teta_13))/2;


double delta2_m21=7.6e-5;
double delta2_m32=2.4e-3;

double delta2_m31_NH=delta2_m32+delta2_m21;
double delta2_m31_IH=delta2_m32-delta2_m21;

double flux_U_235(double E){
    double phi;
    phi=exp(0.870-0.160*E-0.091*pow(E,2));
    return phi;
}

double flux_U_238(double E){
    double phi;
    phi=exp(0.896-0.239*E-0.0981*pow(E,2));
    return phi;
}

double flux_PU_239(double E){
    double phi;
    phi=exp(0.976-0.162*E-0.0790*pow(E,2));
    return phi;
}

double flux_PU_241(double E){//renommer en energy 
    double phi;
    phi=exp(0.793-0.080*E-0.1085*pow(E,2));
    return phi;
}

double flux(double E){
    double phi;
    phi=fission_fraction_U_235*flux_U_235(E)+fission_fraction_U_238*flux_U_238(E)+fission_fraction_PU_239*flux_PU_239(E)+fission_fraction_PU_241*flux_PU_241(E);
    return phi*1e-4;//conversion de m^-2 en cm^-2
}

double sigma (double E){
    double energy_positron=E-(mass_neutron-mass_proton);//Mev
    double moment_positron=sqrt(pow(energy_positron,2)-pow(mass_positron,2));
    return 0.0952e-42*energy_positron*moment_positron;//e-42
}

double probability(double E, char A, bool f){
    double L=53e3;
    double P21=pow(cos(teta_13),4)*pow(sin(2*teta_12),2)*pow(sin(1.27*delta2_m21*L/E),2);
    if(f==0){ return 1-P21;}
    else {
        
        if(A=='N'){
            double P31=pow(cos(teta_12),2)*0.1*pow(sin(1.27*delta2_m31_NH*L/E),2);
            double P32=pow(sin(teta_12),2)*0.1*pow(sin(1.27*delta2_m32*L/E),2);
            return 1-P21-P31-P32;
        }
        else if(A=='I'){
            double P31=pow(cos(teta_12),2)*0.1*pow(sin(1.27*delta2_m31_IH*L/E),2);
            double P32=pow(sin(teta_12),2)*0.1*pow(sin(1.27*(-delta2_m32)*L/E),2);
            return 1-P21-P31-P32;
        } 
    }
    return 0;
}
double energy_positron(double E){
    double energy_positron=E-(mass_neutron-mass_proton);
    double moment_positron=sqrt(pow(energy_positron,2)-pow(mass_positron,2));

    return energy_positron;
}
/*
double neutrino_energy(double Evis){
    double Ev=Evis+0.8; //d'après page 42 article 1
    return Ev;
}*/

double integrale_spectre(double Emin, double Emax,double n){
    double h=(Emax-Emin)/n;
    double result=0;

    for(int i=0;i<n;i++){
        double currentE=Emin+i*h;
        double nextE=Emin+(i+1)*h;

        double prob1=probability(currentE, 'N', 1);
        double sigma1=sigma(currentE);
        double flux1=total_reactor_flux(flux(currentE),36e9);

        double prob2=probability(nextE, 'N', 1);
        double sigma2=sigma(nextE);
        double flux2=total_reactor_flux(flux(nextE),36e9);

        result += h*(prob1*sigma1*flux1 + prob2*sigma2*flux2)/2;
    }
    return result;
}
//use power in MeV
double total_reactor_flux(double tab_flux, double reactor_power){
    double energy_fraction=fission_energy_U_235*fission_fraction_U_235
                            +fission_energy_U_238*fission_fraction_U_238
                            +fission_energy_PU_239*fission_fraction_PU_239
                            +fission_energy_PU_241*fission_fraction_PU_241;//denominator
    double total_flux=reactor_power/(energy_fraction*1.602e-13)*tab_flux;//conversion Mev en j
    return total_flux;
}

double calcul_spectre(double tab_flux,double energy){
    double spectre=tab_flux*sigma(energy)*probability(energy, 'I', 1)/(4*M_PI*pow(53e3,2));//on divise par 4*pi*L^2
    return spectre;
}

double standart_dev(double E){
    double dev=0.03*sqrt(E);
    return dev;
}

double gauss_pdf(double E, double Ei)
{   
    double mu=standart_dev(E);
	return 1.0 / (standart_dev(E) * sqrt(2.0 * M_PI)) * exp(-(pow((E-Ei)/standart_dev(E), 2)/2.0));
}

/*
double convolve(double E1[], double E2, int lenE1, int lenE2){ //int* lenE3){
    int nconv = lenE1 + lenE2 - 1;
    //(*lenE3) = nconv;
    int i, j, E1_start, E2_start, E2_end;
    double E3;
    double *new_E1 = (double*) malloc(size * sizeof(double));

    for(int i = 0; i < size; i++){
            new_E1[i] = gauss_pdf(E[i], E2-0.8);}
    // Pas besoin de réallouer E3 car il est passé en argument
    // double *E3 = (double*) calloc(nconv, sizeof(double));

    for (i = 0; i < nconv; i++)
    {
        E2_start = MAX(0, i - lenE1 + 1);
        E2_end   = MIN(i + 1, lenE2);
        E1_start = MIN(i, lenE1 - 1);
        for(j = E2_start; j < E2_end; j++)
        {
            E3[i] += E1[E1_start--] * E2[j];
        }
    }
    // Pas besoin de retourner E3 car il est passé par référence
    return E3;
}


double* convert_Evis(double E[], int size){
    double *E1 = (double*) malloc(size * sizeof(double));
    
    printf("E[0] vaut %g \n", E[0]);
    printf("gauss de E[0] vaut %g \n", gauss_pdf(E[0], E[0]));

    for(int u = 0; u < 2; u++){
        double *new_E1 = (double*) malloc(size * sizeof(double));
        for(int i = 0; i < size; i++){
            E1[i] = gauss_pdf(E[i]-0.8, E[u]-0.8);
            convolve(E, E1, new_E1, size, size); // Passer E1 et new_E1 par référence
            printf("i  vaut %d \n", i);

        }
        free(E1); // Free previously allocated memory.
        E1[u] = new_E1[u];
    }
    return E1;
}*/
