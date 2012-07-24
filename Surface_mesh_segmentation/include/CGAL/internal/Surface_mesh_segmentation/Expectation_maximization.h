#ifndef CGAL_SEGMENTATION_EXPECTATION_MAXIMIZATION_H
#define CGAL_SEGMENTATION_EXPECTATION_MAXIMIZATION_H
/* NEED TO BE DONE */
/* About implementation:
 * + There are a lot of parameters (with default values) and initialization types, 
 * so I am planning to use whether 'Named Parameter Idiom' or Boost Parameter Library...  
 *
 */
 
#include <vector>
#include <cmath>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <limits>
#include <fstream>
#include <iostream>

#include <CGAL/assertions.h> 
#include <CGAL/internal/Surface_mesh_segmentation/K_means_clustering.h>

#define CGAL_DEFAULT_MAXIMUM_ITERATION 15
#define CGAL_DEFAULT_THRESHOLD 1e-3 
#define CGAL_DEFAULT_NUMBER_OF_RUN 20
#define CGAL_DEFAULT_SEED 1340818006

#define ACTIVATE_SEGMENTATION_EM_DEBUG
#ifdef ACTIVATE_SEGMENTATION_EM_DEBUG
#define SEG_DEBUG(x) x;
#else
#define SEG_DEBUG(x) 
#endif

namespace CGAL {
namespace internal {

class Gaussian_center
{
public:
    double mean;
    double deviation;
    double mixing_coefficient;
    
    Gaussian_center(): mean(0), deviation(0), mixing_coefficient(1.0)
    { }
    Gaussian_center(double mean, double deviation, double mixing_coefficient)
        : mean(mean), deviation(deviation), mixing_coefficient(mixing_coefficient)
    { }
    double probability(double x) const
    {
        double e_over = -0.5 * std::pow((x - mean) / deviation, 2); 
        return exp(e_over) / deviation;
    }
    double probability_with_coef(double x) const
    {
        return probability(x) * mixing_coefficient;
    }
    bool operator < (const Gaussian_center& center) const
    {
        return mean < center.mean;
    }
};
    

class Expectation_maximization
{
public:
    enum Initialization_types { RANDOM_INITIALIZATION, PLUS_INITIALIZATION, K_MEANS_INITIALIZATION };
    
    std::vector<Gaussian_center> centers;
    std::vector<double>  points;

    double threshold;
    int    maximum_iteration;
    
    bool   is_converged;
    double final_likelihood;
    
protected:
    std::vector<std::vector<double> > membership_matrix;
    unsigned int seed; /**< Seed for random initializations */
    Initialization_types init_type;
public:
    // these two constructors will be removed!
    Expectation_maximization() { }    
    
    Expectation_maximization(int number_of_centers, 
        const std::vector<double>& data, 
        Initialization_types init_type = PLUS_INITIALIZATION,
        int number_of_runs = CGAL_DEFAULT_NUMBER_OF_RUN,         
        double threshold = CGAL_DEFAULT_THRESHOLD,
        int maximum_iteration = CGAL_DEFAULT_MAXIMUM_ITERATION )
        :
        points(data), threshold(threshold), maximum_iteration(maximum_iteration), init_type(init_type),
        membership_matrix(std::vector<std::vector<double> >(number_of_centers, std::vector<double>(points.size()))),
        is_converged(false), final_likelihood(-(std::numeric_limits<double>::max)()), seed(CGAL_DEFAULT_SEED)
    {
        /* For initialization with provided center ids per point, with one run */
        if(init_type == K_MEANS_INITIALIZATION)
        {
            K_means_clustering k_means(number_of_centers, data);
            std::vector<int> initial_center_ids;
            k_means.fill_with_center_ids(initial_center_ids);
            
            initiate_centers_from_memberships(number_of_centers, initial_center_ids);
            fit();        
        }
        /* For initialization with random center selection, with multiple run */
        else
        {
            srand(seed);
            fit_with_multiple_run(number_of_centers, number_of_runs);             
        }
        sort(centers.begin(), centers.end());
    }
    
    void fill_with_center_ids(std::vector<int>& data_centers)
    {
        data_centers.reserve(points.size());        
        for(std::vector<double>::iterator point_it = points.begin();
             point_it != points.end(); ++point_it)
        {
            double max_likelihood = 0.0;
            int max_center = -1, center_counter = 0;
            for(std::vector<Gaussian_center>::iterator center_it = centers.begin(); 
                center_it != centers.end(); ++center_it, ++center_counter)
            {
                double likelihood = center_it->probability_with_coef(*point_it);     
                if(max_likelihood < likelihood)
                {
                    max_likelihood = likelihood;
                    max_center = center_counter;
                }
            }
            data_centers.push_back(max_center);
        }
    }   
    
    void fill_with_probabilities(std::vector<std::vector<double> >& probabilities)
    {
        probabilities = std::vector<std::vector<double> >
                        (centers.size(), std::vector<double>(points.size()));
        for(std::size_t point_i = 0; point_i < points.size(); ++point_i)
        {
            double total_probability = 0.0;
            for(std::size_t center_i = 0; center_i < centers.size(); ++center_i)
            {
                double probability = centers[center_i].probability_with_coef(points[point_i]);
                total_probability += probability;
                probabilities[center_i][point_i] = probability;
            }
            for(std::size_t center_i = 0; center_i < centers.size(); ++center_i)
            {
                probabilities[center_i][point_i] /= total_probability;
            }
        }                       
    } 

    //Experimental!
    double calculate_distortion()
    {
        double distortion = 0.0;
        
        for(std::vector<double>::iterator it = points.begin(); it!= points.end(); ++it)
        {
            int closest_center = 0;
            double min_distance = fabs(centers[0].mean - *it);
            for(std::size_t i = 1; i < centers.size(); ++i)
            {
                double distance = fabs(centers[i].mean - *it);
                if(distance < min_distance)
                {
                    min_distance = distance;
                    closest_center = i;
                }
            }
            double distance = (centers[closest_center].mean - *it) / centers[closest_center].deviation;
            distortion += distance * distance;
        }           
        return std::sqrt(distortion / points.size());
    }
    
protected:

    void calculate_initial_deviations()
    {        
        std::vector<int> member_count(centers.size(), 0);
        for(std::vector<double>::iterator it = points.begin(); it!= points.end(); ++it)
        {
            int closest_center = 0;
            double min_distance = fabs(centers[0].mean - *it);
            for(std::size_t i = 1; i < centers.size(); ++i)
            {
                double distance = fabs(centers[i].mean - *it);
                if(distance < min_distance)
                {
                    min_distance = distance;
                    closest_center = i;
                }
            }
            member_count[closest_center]++;
            centers[closest_center].deviation += min_distance * min_distance;
        }
        for(std::size_t i = 0; i < centers.size(); ++i)
        {
            CGAL_assertion(member_count[i] != 0); // There shouldn't be such case, unless same point is selected as a center twice (it is checked!)    
            centers[i].deviation = std::sqrt(centers[i].deviation / member_count[i]);
        }
    }
    // Initialization functions for centers.
 
    void initiate_centers_randomly(int number_of_centers)
    {
        centers.clear();
        /* Randomly generate means of centers */
        double initial_mixing_coefficient = 1.0 / number_of_centers;
        double initial_deviation = 0.0;  
        for(int i = 0; i < number_of_centers; ++i)
        {
            int random_index = rand() % points.size();
            double initial_mean = points[random_index];
            Gaussian_center new_center(initial_mean, initial_deviation, initial_mixing_coefficient);
            // if same point is choosen as a center twice, algorithm will not work
            is_already_center(new_center) ? --i : centers.push_back(new_center); 
        }
        calculate_initial_deviations();
    }
    
    void initiate_centers_plus_plus(int number_of_centers)
    {
        centers.clear();
        double initial_deviation = 0.0;
        double initial_mixing_coefficient = 1.0 / number_of_centers;
        
        std::vector<double> distance_square_cumulative(points.size());
        std::vector<double> distance_square(points.size(), (std::numeric_limits<double>::max)());
        // distance_square stores squared distance to closest center for each point.
        // say, distance_square -> [ 0.1, 0.2, 0.3, 0.4, ... ]
        // then corresponding distance_square_cumulative -> [ 0.1, 0.3, 0.6, 1, ...]
        double initial_mean = points[rand() % points.size()];     
        centers.push_back(Gaussian_center(initial_mean, initial_deviation, initial_mixing_coefficient));
        
        for(int i = 1; i < number_of_centers; ++i)
        {
            double cumulative_distance_square = 0.0;
            for(std::size_t j = 0; j < points.size(); ++j)
            {
                double new_distance = std::pow(centers.back().mean - points[j], 2);
                if(new_distance < distance_square[j]) { distance_square[j] = new_distance; }
                cumulative_distance_square += distance_square[j];
                distance_square_cumulative[j] = cumulative_distance_square;
            }
            
            double random_ds = (rand() / static_cast<double>(RAND_MAX)) * (distance_square_cumulative.back());
            int selection_index = lower_bound(distance_square_cumulative.begin(), distance_square_cumulative.end(), random_ds) 
                - distance_square_cumulative.begin();
            double initial_mean = points[selection_index]; 
            Gaussian_center new_center(initial_mean, initial_deviation, initial_mixing_coefficient);
            // if same point is choosen as a center twice, algorithm will not work
            is_already_center(new_center) ? --i : centers.push_back(new_center); 
        }
        calculate_initial_deviations();
    }
    
    bool is_already_center(const Gaussian_center& center) const
    {
        for(std::vector<Gaussian_center>::const_iterator it = centers.begin(); it != centers.end(); ++it)
        {
            if(it->mean == center.mean) { return true; }
        }
        return false;
    }
    
    void initiate_centers_from_memberships(int number_of_centers, std::vector<int>& initial_center_ids)
    {  
        /* Calculate mean */
        int number_of_points = initial_center_ids.size();
        centers = std::vector<Gaussian_center>(number_of_centers);
        std::vector<int> member_count(number_of_centers, 0);
        
        for(int i = 0; i < number_of_points; ++i)
        {
            int center_id = initial_center_ids[i];
            centers[center_id].mean += points[i];
            member_count[center_id] += 1;
        }
        /* Assign mean, and mixing coef */
        for(int i = 0; i < number_of_centers; ++i)
        {               
            centers[i].mean /= member_count[i];
            centers[i].mixing_coefficient =  member_count[i] / static_cast<double>(number_of_points);
        }
        /* Calculate deviation */
        for(int i = 0; i < number_of_points; ++i)
        {
            int center_id = initial_center_ids[i];
            centers[center_id].deviation += std::pow(points[i] - centers[center_id].mean, 2);
        }    
        for(int i = 0; i < number_of_centers; ++i)
        {
            CGAL_assertion(member_count[i] != 0); // There should be no such case, each center should have at least one member.
            centers[i].deviation = std::sqrt(centers[i].deviation / member_count[i]);
        }
    }
    
    //Main steps of EM
    
    // M step 
    void calculate_parameters()
    {
        for(std::size_t center_i = 0; center_i < centers.size(); ++center_i)
        {
            // Calculate new mean 
            double new_mean = 0.0, total_membership = 0.0;
            for(std::size_t point_i = 0; point_i < points.size(); ++point_i)
            {
                double membership = membership_matrix[center_i][point_i];
                new_mean += membership * points[point_i];
                total_membership += membership;
            }
            new_mean /= total_membership;
            // Calculate new deviation 
            double new_deviation = 0.0;
            for(std::size_t point_i = 0; point_i < points.size(); ++point_i)
            {
                double membership = membership_matrix[center_i][point_i];
                new_deviation += membership * std::pow(points[point_i] - new_mean, 2);
            }
            new_deviation = std::sqrt(new_deviation/total_membership);
            // Assign new parameters 
            centers[center_i].mixing_coefficient = total_membership / points.size();
            centers[center_i].deviation = new_deviation;
            centers[center_i].mean = new_mean;
        }
    }
    
    // Both for E step, and likelihood step
    double calculate_likelihood()
    {
        /** 
         * Calculate Log-likelihood 
         * The trick (merely a trick) is while calculating log-likelihood, we also store probability results into matrix,
         * so that in next iteration we do not have to calculate them again.
         */
        double likelihood = 0.0;
        for(std::size_t point_i = 0; point_i < points.size(); ++point_i)
        {
            double total_membership = 0.0;
            for(std::size_t center_i = 0; center_i < centers.size(); ++center_i)
            {
                double membership = centers[center_i].probability_with_coef(points[point_i]);
                total_membership += membership;
                membership_matrix[center_i][point_i] = membership;
            }
            for(std::size_t center_i = 0; center_i < centers.size(); ++center_i)
            {
                membership_matrix[center_i][point_i] /= total_membership;
            }
            likelihood += log(total_membership);
        }
        return likelihood;
    }
    
    // One iteration of EM
    double iterate(bool first_iteration)
    {
        // E-step
        if(first_iteration) { calculate_likelihood(); }
        // M-step 
        calculate_parameters();
        // Likelihood step 
        return calculate_likelihood();
    }
    
    // Fitting function, iterates until convergence occurs or maximum iteration limit is reached
    double fit()
    {
         double likelihood = -(std::numeric_limits<double>::max)(), prev_likelihood;
         int iteration_count = 0;
         is_converged = false;
         while(!is_converged && iteration_count++ < maximum_iteration) 
         {            
             prev_likelihood = likelihood;
             likelihood = iterate(iteration_count == 1);
             double progress = likelihood - prev_likelihood;
             is_converged = progress < threshold * std::fabs(likelihood);             
         }
         SEG_DEBUG(std::cout << "likelihood: " <<  likelihood << "iteration: " << iteration_count << std::endl)
         if(final_likelihood < likelihood) { final_likelihood = likelihood; }
         return likelihood;
    }
    
    // Calls fit() number_of_run times, and stores best found centers
    void fit_with_multiple_run(int number_of_centers, int number_of_run)
    {
        std::vector<Gaussian_center> max_centers;
        
        while(number_of_run-- > 0)
        {
            init_type == RANDOM_INITIALIZATION ? initiate_centers_randomly(number_of_centers) 
                                               : initiate_centers_plus_plus(number_of_centers);
            
            double likelihood = fit();
            if(likelihood == final_likelihood) { max_centers = centers; }
        }
        SEG_DEBUG(std::cout << "max likelihood: " << final_likelihood << std::endl)
        centers = max_centers;
        //write_center_parameters("centers_param.txt");
    }
    
    void write_random_centers(const char* file_name)
    {
        //std::ofstream output(file_name, std::ios_base::app);
        std::ofstream output(file_name);
        for(std::vector<Gaussian_center>::iterator center_it = centers.begin(); 
            center_it != centers.end(); ++center_it)
        {
            output << center_it->mean << std::endl;           
        }         
        output.close();
    }
    
    void write_center_parameters(const char* file_name)
    {
        //std::ofstream output(file_name, std::ios_base::app);
        std::ofstream output(file_name);
        for(std::vector<Gaussian_center>::iterator center_it = centers.begin(); 
            center_it != centers.end(); ++center_it)
        {
            output << "mean: " << center_it->mean << " dev: " << center_it->deviation 
                << " mix: " << center_it->mixing_coefficient << std::endl;           
        }
    }
};
}//namespace internal
}//namespace CGAL
#undef CGAL_DEFAULT_SEED
#undef CGAL_DEFAULT_MAXIMUM_ITERATION
#undef CGAL_DEFAULT_THRESHOLD 
#undef CGAL_DEFAULT_NUMBER_OF_RUN

#ifdef SEG_DEBUG
#undef SEG_DEBUG
#endif
#endif //CGAL_SEGMENTATION_EXPECTATION_MAXIMIZATION_H