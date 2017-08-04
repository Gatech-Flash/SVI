// [[Rcpp::depends(RcppArmadillo)]]
#include<RcppArmadillo.h>
#include <string>
#include <map>
#include<Rinternals.h>
#include<math.h>
using namespace std;
using namespace Rcpp;
// [[Rcpp::export]]
SEXP SVI_LAD(Rcpp:: List &X,
             SEXP  KK,
             SEXP  nn,
             SEXP alphaa ,
             SEXP  etaa,
             SEXP  pree)
{
    int K = Rcpp::as<int>(KK);
    int n = Rcpp::as<int>(nn);
    double alpha = Rcpp::as<double>(alphaa);
    double eta = Rcpp::as<double>(etaa);
    double pre = Rcpp::as<double>(pree);
    std::map<std::string, int> mymap;
    std::map<std::string, int>::iterator iter;
    for(int i= 0;i< X.size();i++) //Produce map to stroe the whole vocabulary
    {
      Rcpp::StringVector document = Rcpp::as<Rcpp::StringVector>(X[i]) ;
      for(int j=0;j< document.size();j++)
      {
        std::cout<<document[j]<<std::endl;
        mymap[""+document[j]+""]=0;
      }
    }
    int value =0;
    for(iter=mymap.begin();iter !=mymap.end();iter++)
    {
      mymap[""+iter->first+""]=value;
      value++;
    }
     int D= X.size();
     int V= mymap.size();
     Rcpp::NumericMatrix Lambda(V,K);
     for( int i=0;i<V;i++)//initiate Lambda with rchisq
     {
       for(int j=0;j<K;j++)
       {
         Lambda(i,j)=R::rchisq(1);
       }
     }
     Rcpp::NumericMatrix gamma(K,D) ; //random initial with all 0 entry
     std::cout<< "V: "<<V<<std::endl;
     // for( int i=0;i<V;i++)
     // {
     //   for(int j=0;j<K;j++)
     //   {
     //    std::cout<< Lambda(i,j)<<std::endl;
     //   }
     // }
     Rcpp::List phi(D);
     for(int i=0;i<n;i++)
     {
       for(int d=0;d<D;d++)
       {
          for(int v=0;v<K;v++)
          {
            gamma(v,d)=1;
          }
          Rcpp::NumericVector gamma1(K);
          Rcpp::StringVector Y = Rcpp::as<Rcpp::StringVector>(X[d]) ;
          Rcpp::NumericMatrix w(V,Y.size()) ;
          Rcpp::NumericMatrix mid(Y.size(),K);
          Rcpp::NumericMatrix mid_norm(Y.size(),K);
          double norm = sum(abs(gamma(_,d)-gamma1));//calculate norm as stop criteria
        //  Rcpp::LogicalVector test  = (gamma1==gamma1);
          while(norm>=pre)
          {
            gamma1=gamma(_,d);
            for(int j=0;j<Y.size();j++)
            {
              Rcpp::NumericVector w_temp(V);
              iter =mymap.find(""+Y[j]+"");
              int index_inmap = iter->second;
              w_temp[index_inmap]=1;
              w(_,j)= w_temp;
              mid(j,_)=exp(digamma(gamma(_,d))+digamma(Lambda(index_inmap,_))-digamma(colSums(Lambda)));
              mid_norm(j,_)=mid(j,_)/sum(mid(j,_));//normalize parameter
            }
            gamma(_,d)= alpha+colSums(mid_norm);
            norm = sum(abs(gamma(_,d)-gamma1));
          }
           phi[d]= mid_norm;
           arma::mat midnorm_temp = as<arma::mat>(mid_norm);//convert NumericMtrix into arma::mat;
           arma::mat w_temp = as<arma::mat>(w);
           arma::mat result_temp=eta+D*w_temp*midnorm_temp;
           arma::mat Lambda_temp = as<arma::mat>(Lambda);
           Lambda_temp = (1-((i-1)*D+d))*Lambda_temp+(((i-1)*D+d))*result_temp;
           Lambda=Rcpp::wrap(Lambda_temp);
       }
     }
  for(iter=mymap.begin();iter !=mymap.end();iter++)
  {
    std::cout<<iter->first<<"  "<<iter->second<<std::endl;
  }
  return Rcpp::List::create(Lambda,phi,gamma);
}
/*** R

  */
