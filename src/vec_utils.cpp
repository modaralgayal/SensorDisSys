#include <iostream>
#include <vector>
#include <sstream>
#include "vec_utils.h"


std::vector<double> calc_lambda_running_sum (std::vector<double> payload, 
    
    std::vector<double> m_weights, double step_size) {
        // std::cout << "PASSED STEP SIZE: " << step_size << "\n";

  
        try
        {
            // std::cout << "New weights: ";
            for (int i = 0; i < m_weights.size(); i++) {
                m_weights[i] += step_size  * (payload[i] - m_weights[i]);
                std::cout << m_weights[i] << " ";
            }
            std::cout << "\n";

        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << " Error in calc_lambda_running_sum" << '\n';
        }
        
        return m_weights;
}

double sum_elements(std::vector<double> vec) {

    try
    {
        double sum = 0; 
        for (auto val : vec) {
            sum += val;
        }
        return sum; 
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << " Error in sum_elements" << '\n';
    }      
}

std::vector<double> convert_to_vec (std::string& payl) {

        try
        {
            
            if (payl.size() <= 2) {
                return {};
            }

            std::vector<double> newVec; 
            std::stringstream ss(payl);
            std::string token; 
            

            while(std::getline(ss, token, ',')) {
                const std::string trim_chars = " \t\r\n[]";
                size_t start = token.find_first_not_of(trim_chars);
                if (start == std::string::npos) continue;  
                size_t last = token.find_last_not_of(trim_chars);
                token = token.substr(start, last-start+1);
                
                // std::cout << "This is the token: " << token << "\n";
                newVec.push_back(std::stod(token));
            }

            return newVec;
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << " Error in convert_to_vec" << '\n';
        }
    };