#include "vec_utils.h"


std::vector<double> calc_lambda_running_sum (std::vector<double> payload, 
    std::vector<double> weights, double step_size) {
        std::cout << "PASSED STEP SIZE: " << step_size << "\n";


        try
        {
            std::cout << "New weights: ";
            for (int i = 0; i < weights.size(); i++) {
                weights[i] += step_size  * (payload[i] - weights[i]);
                std::cout << weights[i] << " ";
            }
            std::cout << "\n";

        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << " Error in calc_lambda_running_sum" << '\n';
            return {};
        }
        
        return weights;
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
        return {};
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
            return {};
        }
    };