//
// Created by Vetle Wegner Ingeberg on 16/04/2021.
//

#ifndef TDR2TREE_TIMEVAL_H
#define TDR2TREE_TIMEVAL_H

struct time_val_t {
    int64_t timestamp;
    double cfd_corr;
};

inline double operator-(const time_val_t &lhs, const time_val_t &rhs){
    return double( lhs.timestamp - rhs.timestamp ) + lhs.cfd_corr - rhs.cfd_corr;
}

#endif //TDR2TREE_TIMEVAL_H
