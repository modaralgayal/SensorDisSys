#ifndef VEC_UTILS_H

#define VEC_UTILS_H
#include <vector>
#include <string>
#include <iostream>
#include <sstream>

std::vector<double> calc_lambda_running_sum (std::vector<double> payload, 
        std::vector<double> m_weights, double step_size);
double sum_elements(std::vector<double> vec);

std::vector<double> convert_to_vec (std::string& payl);
#endif