/*
* Code for all the simulations with nice descriptions, this code is described as a pseudocode.  Change the extent of the boundary. With tunnels
*/


#include "Aboria.h"
#include <random>
#include <map>
#include <Eigen/Core>
#include <algorithm> // std::unique_copy
#include <iostream>// writing on a text file
#include <fstream>
#include <math.h>
#include <assert.h>

using namespace std;
using namespace Aboria;
using namespace Eigen; // objects VectorXf, MatrixXf

VectorXi proportions(double diff_conc, int n_seed) {


    // model parameters

    /*
     * Initial length of the domain is 300 \mu m, I will create a matrix of chemoattractant which will be of length 300/10= 30.
     * All the length parameters will be scaled consistently, i.e. /10.
     *
     */

    int length_x = 30; // length in x direction of the chemoattractant matrix
    double domain_length = 30; //this variable is for the actual domain length, since it will be increasing
    double old_length = 30;// this is important for the update of the positions of cells
    const int length_y = 12; // length in y direction of the chemoattractant matrix
    double cell_radius = 0.75;//0.5; // radius of a cell
    const double diameter =
            2 * cell_radius; // diameter of a cell
    const int N_steps = 1440; // number of timesteps, 1min - 1timestep, from 6h tp 24hours.
    const size_t N = 5; // initial number of cells
    double l_filo_y = 2.75;//2; // sensing radius, filopodia + cell radius
    double l_filo_x = 2.75; // sensing radius, it will have to be rescaled when domain grows
    double l_filo_x_in = l_filo_x; // this value is used for rescaling when domain grows based on initial value
    double l_filo_max = 4.5; // this is the length when two cells which were previously in a chain become dettached
    //double diff_conc = 0.1; // sensing threshold, i.e. how much concentration has to be bigger, so that the cell moves in that direction
    int freq_growth = 1; // determines how frequently domain grows (actually not relevant because it will go every timestep)
    int insertion_freq = 1; // determines how frequently new cells are inserted, regulates the density of population
    double speed_l = 0.14;// 0.05;//1;//0.05; // speed of a leader cell
    double increase_fol_speed = 1.3;
    double speed_f = increase_fol_speed * speed_l;//0.05;//0.1;//0.08; // speed of a follower cell
    double dettach_prob = 0.5; // probability that a follower cell which is on trail looses the trail
    double chemo_leader = 0.9; //0.5; // phenotypic switching happens when the concentration of chemoattractant is higher than this (presentation video 0.95), no phenotypic switching
    double eps = 1; // for phenotypic switching, the distance has to be that much higher
    const int filo_number = 2; // number of filopodia sent
    int same_dir = 0; // number of steps in the same direction +1, because if 0, then only one step in the same direction
    bool random_pers = true; // persistent movement also when the cell moves randomly
    int count_dir = 0; // this is to count the number of times the cell moved the same direction, up to same_dir for each cell


    // distance to the track parameters
    double dist_thres = 1;
    int closest_time;
    int leader_track;
    double track_spacing = 2; // spacing between positions on the track
    int track_length = 60;


    // domain growth parameters

//    double L_0 = 30; // will have to make this consistent with actual initial length
//    double a = 0.001;//0.008;//0.23/10;
//    double L_inf = 86.76;
//    double t_s = 16;//4.31*10;
//    double constant = 29.12;


//    double L_0 = 30;
//    double a =0.23;
//    double L_inf = 86.76;
//    double t_s = 15.9; // think about these rescaled variables
//    double constant = 29.12 ;

    // for 24 hours

    double L_0 = 30;
    double a =0.288;
    double L_inf = 86.76;
    double t_s = 12.77; // think about these rescaled variables
    double constant = 29.12 ;




    double domain_len_der = 0; // initialise derivative of the domain growth function


    // parameters for the dynamics of chemoattractant concentration

    double D = 0.0001; // to 10^5 \nu m^2/h diffusion coefficient
    double t = 0; // initialise time, redundant
    double dt = 1; // time step
    double dt_init = dt;
    int number_time = int(1/dt_init); // how many timesteps in 1min, which is the actual simulation timestep
    double dx = 1; // space step in x direction, double to be consistent with other types
    double dy = 1; // space step in y direction
    double kai = 0.0001;//0;//0.1 // to 1 /h production rate of chemoattractant


    // parameters for internalisation

    double R = cell_radius;//7.5/10; // \nu m cell radius
    double lam = 0.00035;//(100)/10; // to 1000 /h chemoattractant internalisation

    //Boundary
    bool boundary = true; // if true full boundary, if false short boundary
    double boundlen = 0.0; // if boundary is false, then this is the lenght of hard boundary, write 20 for 200
    double chemooutside = 0.0;
    /*
     * initialise a matrix that stores values of concentration of chemoattractant
     * */

    MatrixXf chemo = MatrixXf::Zero(length_x, length_y);
    MatrixXf chemo_new = MatrixXf::Zero(length_x, length_y);

    // initialise internalisation matrix
    MatrixXf intern = MatrixXf::Zero(length_x, length_y);

    for (int i = 0; i < length_x; i++) {
        for (int j = 0; j < length_y; j++) {
            chemo(i, j) = 1; // uniform concentration initially
            chemo_new(i, j) = 1; // this is for later updates
        }
    }


    // four columns for x, y, z, u (z is necessary for paraview)

    // form a matrix which would store x,y,z,u

    MatrixXf chemo_3col(length_x * length_y, 4), chemo_3col_ind(length_x * length_y,
                                                                2); // need for because that is how paraview accepts data, third dimension is just zeros

    // x, y coord, 1st and 2nd columns respectively
    int k = 0;
    // it has to be 3D for paraview
    while (k < length_x * length_y) {
        for (int i = 0; i < length_x; i++) {
            for (int j = 0; j < length_y; j++) {
                chemo_3col_ind(k, 0) = i;
                chemo_3col_ind(k, 1) = j;
                chemo_3col(k, 2) = 0;
                k += 1;
            }
        }
    }


    // y and x (initially) column
    for (int i = 0; i < length_x * length_y; i++) {
        chemo_3col(i, 1) = chemo_3col_ind(i, 1);
        chemo_3col(i, 0) = chemo_3col_ind(i, 0);
    }

    // create an array to keep track of all the positions of leader cells

    vdouble2 track_position [track_length][N] = {vdouble2(0,0)};
    for (int i =0; i < track_length; i++){
        for(int j = 0; j < N; j++){
            track_position[i][j] = vdouble2(0,0);
        }
    }
    int track_time[N] = {0}; // vector that stores the time values for each leader when there was a sufficiently big change in the position



    /*
     * 2D domain with a few randomly placed particles
     */

    /*
     * initial cells of fixed radius
     */

    //ABORIA_VARIABLE(velocity,vdouble2,"velocity")
    ABORIA_VARIABLE(radius, double, "radius")
    ABORIA_VARIABLE(direction, vdouble2, "direction")// stores the direction a particle moved
    ABORIA_VARIABLE(persistence_extent, int, "persistence extent")// stores whether cell moves only one step in current direction or in a process of moving persistently
    ABORIA_VARIABLE(same_dir_step, int, "same dir step")// the number which stores how many steps in the same direction are made.
    ABORIA_VARIABLE(attached_to_id, int, "attached_to_id")
    ABORIA_VARIABLE(type, int, "type") // 0 if a cell is a leader, 1 if follower
    ABORIA_VARIABLE(chain_type, int, "chain_type") // leaders form different chain types
    ABORIA_VARIABLE(chain, int, "chain") // stores whether a follower is part of the chain or no, 0 if it is not part of
    // the chain and then increasing integer if it is. If it is attached to a leader, it is 1, and then increasing order.

    // tunneling model
    ABORIA_VARIABLE(attached_at_time_step,int,"attached_at_time_step")
    ABORIA_VARIABLE(attached_leader_nr,int,"attached_leader_nr")
    ABORIA_VARIABLE(in_track, int, "in_track") // stores whether attached to a leader or follower
    // the chain and then increasing integer if it is. If it is attached to a leader, it is 1, and then increasing order.
    // stores the distance to the closest neighbour, if less than thresold
    typedef Particles<std::tuple<radius, type, attached_to_id, attached_at_time_step,in_track, attached_leader_nr, direction, chain, chain_type, persistence_extent, same_dir_step>, 2> particle_type; // 2 stands for dimension

    // will use stored value of the position of a particle
    typedef particle_type::position position;

    // initialise the number of particles
    particle_type particles(N);

    // initialise random number generator for particles entering the domain, appearing at the start in x and uniformly in y
    std::default_random_engine gen;
    std::uniform_real_distribution<double> uniform(cell_radius, length_y-1 - cell_radius);


    /*
     * compact initialisation of particles
     */

    for (int i = 0; i < N; ++i) {


        get<radius>(particles[i]) = cell_radius;
        get<type>(particles[i]) = 0; // initially all cells are leaders

        //get<position>(p) = vdouble2(cell_radius,(i+1)*diameter); // x=2, uniformly in y
        get<position>(particles[i]) = vdouble2(cell_radius, (i + 1) * double(length_y-1 - cell_radius) / double(N) -
                                                            0.5 * double(length_y-1 - cell_radius) /
                                                            double(N)); // x=2, uniformly in y
        get<persistence_extent>(particles[i]) = 0;
        get<same_dir_step>(particles[i]) = 0;

    }

    // initialise neighbourhood search, note that the domain will grow in x direction, so I initialise larger domain
    particles.init_neighbour_search(vdouble2(-50, -50), 5 * vdouble2(length_x, length_y), vbool2(false, false));

    // save particles before they move

    // vtkWriteGrid("LEADFOLFullBoundaryCells", t, particles.get_grid(true));

    // initialise random number generator to obtain random number between 0 and 2*pi
    std::default_random_engine gen1;
    gen1.seed(t*n_seed); // choose different seeds to obtain different random numbers
    std::uniform_real_distribution<double> uniformpi(0, 2 * M_PI);

    //for each timestep
    for (int t = 0; t < N_steps; t++) {



        // insert new cells at the start of the domain at insertion time (have to think about this insertion time)

        if (t % insertion_freq == 0) {
            bool free_position = false;
            particle_type::value_type f;
            //get<radius>(f) = cell_radius;


            get<position>(f) = vdouble2(cell_radius, uniform(gen)); // x=2, uniformly in y
            free_position = true;
            /*
             * loop over all neighbouring leaders within "dem_diameter" distance
             */
            for (auto tpl = euclidean_search(particles.get_query(), get<position>(f), diameter); tpl != false; ++tpl) {

                vdouble2 diffx = tpl.dx();

                if (diffx.norm() < diameter) {
                    free_position = false;
                    break;
                }
            }

            // our assumption that all new cells are followers
            get<type>(f) = 1;


            if (free_position) {
                get<chain>(f) = 0;
                get<chain_type>(f) = -1;
                get<attached_to_id>(f) = -1;
                particles.push_back(f);
            }

        }
        particles.update_positions();

        /*
         * Domain growth
         * */



        for (int ka =0; ka< number_time;ka++){


            domain_length = ((L_inf * exp(a * (dt/60.0 - t_s))) / (L_inf / L_0 + exp(a * (dt/60.0 - t_s)) - 1)) + constant;

            domain_len_der = ((a * L_inf/60 * exp(a * (dt/60.0 - t_s))) / (L_inf / L_0 + exp(a * (dt/60.0 - t_s)) - 1) -
                              (a * L_inf/60 * exp(2 * a * (dt/60.0 - t_s))) / ((L_inf / L_0 + exp(a * (dt/60.0 - t_s)) - 1)*(L_inf / L_0 + exp(a * (dt/60.0 - t_s)) - 1)));


//            domain_length = ((L_inf * exp(a * (dt - t_s))) / (L_inf / L_0 + exp(a * (dt - t_s)) - 1)) + constant;
//
//            domain_len_der = ((a * L_inf * exp(a * (dt - t_s))) / (L_inf / L_0 + exp(a * (dt - t_s)) - 1) -
//                              (a * L_inf * exp(2 * a * (dt - t_s))) / ((L_inf / L_0 + exp(a * (dt - t_s)) - 1)*(L_inf / L_0 + exp(a * (dt - t_s)) - 1)));



            // internalisation
            for (int i = 0; i < length_x; i++) {
                for (int j = 0; j < length_y; j++) {
                    //go through all the cells
                    for (int k = 0; k < particles.size(); k++) {
                        // leaders
                        //for (int k = 0; k < N; k++) {
                        vdouble2 x;
                        x = get<position>(particles[k]);
                        intern(i, j) = intern(i, j) + exp(-(((domain_length / length_x) * i - x[0]) *
                                                            ((domain_length / length_x) * i - x[0]) +
                                                            (j - x[1]) * (j - x[1])) /
                                                          (2 * R * R)); // mapping to fixed domain
                    }
                }
            }



            // timestep is 1
            //while (dt <= 1) {

            // reaction-diffusion equation for chemoattractant
            for (int i = 1; i < length_x - 1; i++) {
                for (int j = 1; j < length_y - 1; j++) {


                    // logistic production rate

                    chemo_new(i, j) = dt_init * (D * ((1 / ((domain_length / length_x) * (domain_length / length_x))) *
                                                      (chemo(i + 1, j) - 2 * chemo(i, j) + chemo(i - 1, j)) / (dx * dx) +
                                                      (chemo(i, j + 1) - 2 * chemo(i, j) + chemo(i, j - 1)) / (dy * dy)) -
                                                 (chemo(i, j) * lam / (2 * M_PI * R * R)) * intern(i, j) +
                                                 kai*(chemo(i,j)) -
                                                 double(domain_len_der) / double(domain_length) * chemo(i, j)) + chemo(i, j);


                }
            }

            dt += dt_init;



            // zero flux boundary conditions

            for (int i = 0; i < length_y; i++) {
                chemo_new(0, i) = chemo_new(1, i);
                chemo_new(length_x - 1, i) = chemo_new(length_x - 2, i);

            }

            for (int i = 0; i < length_x; i++) {
                chemo_new(i, 0) = chemo_new(i, 1);
                chemo_new(i, length_y - 1) = chemo_new(i, length_y - 2);
            }


            chemo = chemo_new; // update chemo concentration
            //}


        }



        // save the chemoattractant concentration with properly rescaled coordinates
        for (int i = 0; i < length_x * length_y; i++) {
            chemo_3col(i, 0) = chemo_3col_ind(i, 0) * (domain_length / length_x);
        }


        // u column
        for (int i = 0; i < length_x * length_y; i++) {
            chemo_3col(i, 3) = chemo(chemo_3col_ind(i, 0), chemo_3col_ind(i, 1));
        }


        // update chemoattractant profile after the domain grew






        //}







        // save data to plot chemoattractant concentration

        if (t%100 == 0 ){
            ofstream output("M11matrix" + to_string(t) + ".csv");

            output << "x, y, z, u" << "\n" << endl;


            for (int i = 0; i < length_x * length_y; i++) {
                for (int j = 0; j < 4; j++) {
                    output << chemo_3col(i, j) << ", ";
                }
                output << "\n" << endl;
            }
        }



        /// update positions uniformly based on the domain growth

        if (t % freq_growth == 0) {

            for (int i = 0; i < particles.size(); i++) {
                get<position>(particles)[i] *= vdouble2((domain_length / old_length), 1);
            }
            old_length = domain_length;
        }

        /*
         * Update the position of particles
         * */


        //  create a random list of cell ids
        int check_rep = 0; // check for repetitions, 0 no rep, 1 rep


        std::default_random_engine gen2;
        gen2.seed(t*n_seed); // different seeds
        std::uniform_real_distribution<double> uniform_particles(0, particles.size()); // can only move forward

        VectorXi particle_id = VectorXi::Zero(particles.size());

        for (int i = 0; i < particles.size(); i++) {

            check_rep = 1; // set to 1 to enter the while loop
            while (check_rep == 1) {
                check_rep = 0; // it is initially zero and then will be changed to 1 if it is equivalent to others
                particle_id(i) = uniform_particles(gen2);


                for (int j = 0; j < i; j++) {
                    if (particle_id(i) == particle_id(j)) { check_rep = 1; }
                }
            }
        }

        // update the position of all particles in a random order created above

        for (int j = 0; j < particles.size(); j++) {

            /*
            * phenotypic switching, based on chemoattractant concentration in front, +0.5
            */
//
//            vdouble2 coord = get<position>(particles[particle_id(j)]);
//
//            // rescaled coord
//
//            double rescaled_coord;
//
//            rescaled_coord = (length_x / domain_length)*coord[0];
//
//            double chemo_in_front = chemo(round(rescaled_coord), round(coord[1]));
//            //cout << "chemo in front " << old_chemo << endl;
//
//
//            // if high concentration cells become leaders
//            if (chemo_in_front > chemo_leader ){
//                get<type>(particles[particle_id(j)]) = 0;
//            }
//            else{
//                get<type>(particles[particle_id(j)]) = 1;
//            }

            // if a particle is a leader
            if (get<type>(particles[particle_id(j)]) == 0) {

                vdouble2 x; // use variable x for the position of cells
                x = get<position>(particles[particle_id(j)]);

                double x_in; // x coordinate in initial domain length scale
                x_in = (length_x / domain_length) * x[0];
                l_filo_x = (length_x / domain_length) * l_filo_x_in; // rescale the length of filopodia as well




                // if it is still in the process of moving in the same direction
                if (get<persistence_extent>(particles[particle_id(j)]) == 1) {


                    x += get<direction>(particles[particle_id(j)]);

                    x_in = (length_x / domain_length) * x[0];// scale to initial coordinates

                    bool free_position = true; // check if the neighbouring position is free

                    // check if there are other particles in the position where the particle wants to move
                    for (auto k = euclidean_search(particles.get_query(), x, diameter); k != false; ++k) {
                        if (get<id>(*k) != get<id>(particles[particle_id(j)])) { // check if it is not the same particle
                            free_position = false;
                        }
                    }

                    // check that the position they want to move to is free and not out of bounds


                    // full length boundary
                    if (boundary == true){
                        if (free_position && x[0] > cell_radius && (x[1]) > cell_radius &&
                            (x[1]) < length_y-1 - cell_radius) {
                            // if that is the case, move into that position
                            get<position>(particles)[particle_id(j)] +=
                                    get<direction>(particles)[particle_id(j)];
                        }
                    }


                    // SHORT BOUNDARY

                    // hard boundary if the position is less than boundlen
                    if (boundary == false){
                        if (x[0] < boundlen){
                            if (free_position && x[0] > cell_radius && (x[1]) > cell_radius &&
                                (x[1]) < length_y-1 - cell_radius) {
                                // if that is the case, move into that position
                                get<position>(particles)[particle_id(j)] +=
                                        get<direction>(particles)[particle_id(j)];
                            }
                        }          // no boundary in y if the position is after boundlen
                        else{
                            if (free_position && x[0] > cell_radius) {
                                // if that is the case, move into that position
                                get<position>(particles)[particle_id(j)] +=
                                        get<direction>(particles)[particle_id(j)];
                            }

                        }
                    }



                    get<same_dir_step>(particles)[particle_id(
                            j)] += 1; // add regardless whether the step happened or no to that count of the number of movement in the same direction

                }


                // if a particle is not in a sequence of persistent steps
                if (get<persistence_extent>(particles[particle_id(j)]) == 0) {



                    // create an array to store random directions
                    array<double, filo_number + 1> random_angle;

                    // choose the number of angles where the filopodia is sent
                    for (int k = 0; k < filo_number + 1; k++) {

                        double random_angle_tem = uniformpi(gen1);
                        int sign_x_tem, sign_y_tem;

                        random_angle_tem = uniformpi(gen1);
                        random_angle[k] = random_angle_tem;

                    }


                    // choose which direction to move


                    // store variables for concentration at new locations
                    double old_chemo;
                    array<double, filo_number> new_chemo;
                    // if the cell is inside the domain
                    if (round(x)[1] > 0 && round(x)[1] < length_y-1){
                        old_chemo = chemo((round(x_in)), round(x)[1]);


                        for (int i = 0; i < filo_number; i++) {

                            if (round((x_in + sin(random_angle[i]) * l_filo_x)) < 0 ||
                                round((x_in + sin(random_angle[i]) * l_filo_x)) >
                                length_x - 1 || round(x[1] + cos(random_angle[i]) * l_filo_y) < 0 ||
                                round(x[1] + cos(random_angle[i]) * l_filo_y) > length_y - 1) {
                                new_chemo[i] = chemooutside;
                            } else {
                                new_chemo[i] = chemo(round((x_in + sin(random_angle[i]) * l_filo_x)),
                                                     round(x[1] + cos(random_angle[i]) * l_filo_y));
                            }

                        }

                    }else{  // if the cell is ouside the domain
                        old_chemo = chemooutside;

                        for (int i = 0; i < filo_number; i++) {

                            if (round((x_in + sin(random_angle[i]) * l_filo_x)) < 0 ||
                                round((x_in + sin(random_angle[i]) * l_filo_x)) >
                                length_x - 1 || round(x[1] + cos(random_angle[i]) * l_filo_y) < 0 ||
                                round(x[1] + cos(random_angle[i]) * l_filo_y) > length_y - 1) {
                                new_chemo[i] = chemooutside;
                            } else {
                                new_chemo[i] = chemo(round((x_in + sin(random_angle[i]) * l_filo_x)),
                                                     round(x[1] + cos(random_angle[i]) * l_filo_y));
                            }

                        }

                    }




                    // find maximum concentration of chemoattractant

                    int chemo_max_number = 0;

                    for (int i = 1; i < filo_number; i++) {
                        if (new_chemo[chemo_max_number] < new_chemo[i]) {
                            chemo_max_number = i;
                        }
                    }

                    // if the concentration in a new place is relatively higher than the old one (diff_conc determines that threshold), move that way
                    if ((new_chemo[chemo_max_number] - old_chemo) / sqrt(old_chemo) > diff_conc) {

                        count_dir += 1;

                        x += speed_l *
                             vdouble2(sin(random_angle[chemo_max_number]), cos(random_angle[chemo_max_number]));

                        x_in = (length_x / domain_length) * x[0];//scale to initial coordinates


                        bool free_position = true; // check if the neighbouring position is free

                        // check if the position the particle wants to move is free
                        for (auto k = euclidean_search(particles.get_query(), x, diameter); k != false; ++k) {

                            if (get<id>(*k) !=
                                get<id>(particles[particle_id(j)])) { // check if it is not the same particle
                                free_position = false;
                            }
                        }


                        // if the position they want to move to is free and not out of bounds, move that direction

                        // full length boundary
                        if (boundary == true)
                            if (free_position && x[0] > cell_radius && (x[1]) > cell_radius &&
                                (x[1]) < length_y-1 - cell_radius) {
                                get<position>(particles)[particle_id(j)] +=
                                        speed_l * vdouble2(sin(random_angle[chemo_max_number]),
                                                           cos(random_angle[chemo_max_number])); // update if nothing is in the next position
                                get<direction>(particles)[particle_id(j)] =
                                        speed_l * vdouble2(sin(random_angle[chemo_max_number]),
                                                           cos(random_angle[chemo_max_number]));

                                // if there is some kind of tendency to move persistently
                                if (same_dir > 0) {
                                    get<persistence_extent>(particles[particle_id(
                                            j)]) = 1; // assume for now that it also becomes peristent in random direction

                                }

                            }


//                        // SHORT BOUNDARY
                        if (boundary == false){
                            // hard boundary if the position is less than boundlen
                            if (x[0] < boundlen){
                                if (free_position && x[0] > cell_radius && (x[1]) > cell_radius &&
                                    (x[1]) < length_y-1 - cell_radius) {
                                    get<position>(particles)[particle_id(j)] +=
                                            speed_l * vdouble2(sin(random_angle[chemo_max_number]),
                                                               cos(random_angle[chemo_max_number])); // update if nothing is in the next position
                                    get<direction>(particles)[particle_id(j)] =
                                            speed_l * vdouble2(sin(random_angle[chemo_max_number]),
                                                               cos(random_angle[chemo_max_number]));

                                    // if there is some kind of tendency to move persistently
                                    if (same_dir > 0) {
                                        get<persistence_extent>(particles[particle_id(
                                                j)]) = 1; // assume for now that it also becomes peristent in random direction

                                    }

                                }
                            }else{// no boundary in y if the position is after boundlen
                                if (free_position && x[0] > cell_radius) {
                                    get<position>(particles)[particle_id(j)] +=
                                            speed_l * vdouble2(sin(random_angle[chemo_max_number]),
                                                               cos(random_angle[chemo_max_number])); // update if nothing is in the next position
                                    get<direction>(particles)[particle_id(j)] =
                                            speed_l * vdouble2(sin(random_angle[chemo_max_number]),
                                                               cos(random_angle[chemo_max_number]));

                                    // if there is some kind of tendency to move persistently
                                    if (same_dir > 0) {
                                        get<persistence_extent>(particles[particle_id(
                                                j)]) = 1; // assume for now that it also becomes peristent in random direction

                                    }

                                }

                            }
                        }




                    }

                        // if the concentration is not higher, move in random direction
                    else {


                        x += speed_l * vdouble2(sin(random_angle[filo_number]), cos(random_angle[filo_number]));

                        x_in = (length_x / domain_length) * x[0];// scale to initial length

                        bool free_position = true; // check if the neighbouring position is free

                        // if this loop is entered, it means that there is another cell where I want to move
                        for (auto k = euclidean_search(particles.get_query(), x, diameter); k != false; ++k) {

                            if (get<id>(*k) !=
                                get<id>(particles[particle_id(j)])) { // check if it is not the same particle
                                free_position = false;
                            }
                        }


                        // update the position if the place they want to move to is free and not out of bounds
                        //full boundary
                        if (boundary == true){
                            if (free_position && x[0] > cell_radius &&  (x[1]) > cell_radius &&
                                (x[1]) < length_y-1 - cell_radius) {
                                get<position>(particles)[particle_id(j)] +=
                                        speed_l * vdouble2(sin(random_angle[filo_number]),
                                                           cos(random_angle[filo_number])); // update if nothing is in the next position
                                get<direction>(particles)[particle_id(j)] =
                                        speed_l * vdouble2(sin(random_angle[filo_number]),
                                                           cos(random_angle[filo_number]));
                                // if particles start moving persistently in all directions
                                if (random_pers) {
                                    if (same_dir > 0) {
                                        get<persistence_extent>(particles[particle_id(
                                                j)]) = 1; // assume for now that it also becomes peristent in random direction

                                    }
                                }

                            }
                        }


                        //Short boundary
                        if (boundary == false){
                            if (x[0] < boundlen ){
                                if (free_position && x[0] > cell_radius &&  (x[1]) > cell_radius &&
                                    (x[1]) < length_y-1 - cell_radius) {
                                    get<position>(particles)[particle_id(j)] +=
                                            speed_l * vdouble2(sin(random_angle[filo_number]),
                                                               cos(random_angle[filo_number])); // update if nothing is in the next position
                                    get<direction>(particles)[particle_id(j)] =
                                            speed_l * vdouble2(sin(random_angle[filo_number]),
                                                               cos(random_angle[filo_number]));
                                    // if particles start moving persistently in all directions
                                    if (random_pers) {
                                        if (same_dir > 0) {
                                            get<persistence_extent>(particles[particle_id(
                                                    j)]) = 1; // assume for now that it also becomes peristent in random direction

                                        }
                                    }

                                }
                            }else{
                                if (free_position && x[0] > cell_radius ) {
                                    get<position>(particles)[particle_id(j)] +=
                                            speed_l * vdouble2(sin(random_angle[filo_number]),
                                                               cos(random_angle[filo_number])); // update if nothing is in the next position
                                    get<direction>(particles)[particle_id(j)] =
                                            speed_l * vdouble2(sin(random_angle[filo_number]),
                                                               cos(random_angle[filo_number]));
                                    // if particles start moving persistently in all directions
                                    if (random_pers) {
                                        if (same_dir > 0) {
                                            get<persistence_extent>(particles[particle_id(
                                                    j)]) = 1; // assume for now that it also becomes peristent in random direction

                                        }
                                    }

                                }
                            }
                        }





                    }

                }

                // check if it is not the end of moving in the same direction
                if (get<same_dir_step>(particles)[particle_id(j)] > same_dir) {
                    get<persistence_extent>(particles)[particle_id(j)] = 0;
                    get<same_dir_step>(particles[particle_id(j)]) = 0;
                }

            }




            // if a particle is a follower
            if (get<type>(particles[particle_id(j)]) == 1) {

                vdouble2 x;
                x = get<position>(particles[particle_id(j)]);

                double x_in; // x coordinate in initial domain length scale
                x_in = (length_x / domain_length) * x[0];

                // if the particle is part of the chain
                if (get<chain>(particles[particle_id(j)]) > 0) {


                    // check if it is not too far from the cell it was following

                    vdouble2 dist;

                    dist = get<position>(particles[particle_id(j)]) -
                           get<position>(particles[get<attached_to_id>(particles[particle_id(j)])]);

                    // if it is sufficiently far dettach the cell
                    if (dist.norm() > l_filo_max) {
                        get<chain>(particles[particle_id(j)]) = 0;
                        //dettach also all the cells that are behind it, so that other cells would not be attached to this chain
                        for (int i = 0; i < particles.size(); ++i) {
                            if (get<chain_type>(particles[i]) == get<chain_type>(particles)[particle_id(j)]) {
                                // get<chain_type>(particles)[i] = -1;
                                get<chain>(particles[i]) = 0;
                            }

                        }
                        // get<chain_type>(particles)[particle_id(j)] = -1;
                    }


                    // direction the same as of the cell it is attached to
                    get<direction>(particles)[particle_id(j)] = get<direction>(particles)[get<attached_to_id>(
                            particles[particle_id(j)])];

                    //try to move in the same direction as the cell it is attached to
                    vdouble2 x_chain = x + increase_fol_speed * get<direction>(particles)[particle_id(j)];

                    double x_in_chain; // scaled coordinate

                    x_in_chain =
                            (length_x / domain_length) * x_chain[0];//uniform growth in the first part of the domain


                    bool free_position = true;

                    // check if the position it wants to move to is free
                    for (auto pos = euclidean_search(particles.get_query(), x_chain, diameter);
                         pos != false; ++pos) {
                        if (get<id>(*pos) !=
                            get<id>(particles[particle_id(j)])) { // check if it is not the same particle
                            free_position = false;
                        }
                    }


                    // update the position if the place they want to move to is free and not out of bounds
                    if (boundary == true){
                        if (free_position && x_chain[0] > cell_radius  && (x_chain[1]) > cell_radius &&
                            (x_chain[1]) < length_y -1- cell_radius) {
                            get<position>(particles)[particle_id(j)] +=
                                    increase_fol_speed * get<direction>(particles[particle_id(j)]);

                        }
                    }

                    if (boundary == false){
                        if (x_chain[0] < boundlen){
                            if (free_position && x_chain[0] > cell_radius  && (x_chain[1]) > cell_radius &&
                                (x_chain[1]) < length_y -1- cell_radius) {
                                get<position>(particles)[particle_id(j)] +=
                                        increase_fol_speed * get<direction>(particles[particle_id(j)]);

                            }
                        }else{
                            if (free_position && x_chain[0] > cell_radius) {
                                get<position>(particles)[particle_id(j)] +=
                                        increase_fol_speed * get<direction>(particles[particle_id(j)]);

                            }
                        }

                    }

//                        /*
//                           * NOT IN THE SUMMARY I SENT THEM
//                           */
//                    if (free_position == false){
//                        get<chain>(particles[particle_id(j)]) = 0;
//                    }





                }

/*
                 * Model with tunnels
                 * */



                if (get<chain>(particles)[particle_id(j)] == 0) {


                    /* check if there are any cells distance l_filo_y apart
                     * it can be either a leader or a follower already in a chain
                    */


                    // try to find a close leader
                    for (auto k = euclidean_search(particles.get_query(), x, l_filo_x_in); k != false; ++k) {


                        if (get<type>(*k) == 0) { // if it is close to a leader
                            get<direction>(particles)[particle_id(j)] = get<direction>(*k); // set the same direction
                            get<chain>(particles)[particle_id(j)] = 1; // note that it is directly attached to a leader
                            get<attached_to_id>(particles)[particle_id(j)] = get<id>(
                                    *k); // note the id of the particle it is attached to
                            get<chain_type>(particles)[particle_id(j)] = get<id>(
                                    *k); // chain type is the id of the leader
                        }

                    }


                    // if it hasn't found a leader nearby,
                    // try to find a close follower which is in a chain contact with a leader

                    if (get<chain>(particles)[particle_id(j)] != 1) {
                        for (auto k = euclidean_search(particles.get_query(), x, l_filo_y); k != false; ++k) {

                            // if it is close to a follower that is part of the chain
                            if (get<type>(*k) == 1 && get<chain>(*k) > 0) {

                                if (get<id>(*k) != get<id>(particles[particle_id(j)])) {
                                    //check if there is a leader in front of the chain
                                    //for (int i= 0; i< particles.size(); i++){//go through all the particles
                                    //  if (get<chain_type>(particles)[i] == get<chain_type>(*k) && get<chain>(particles)[i] == 1){// if they are in the same chain and directly attached to a leader

                                    get<direction>(particles)[particle_id(j)] = get<direction>(*k);
                                    get<chain>(particles)[particle_id(j)] =
                                            get<chain>(*k) + 1; // it is subsequent member of the chain
                                    get<attached_to_id>(particles)[particle_id(j)] = get<id>(
                                            *k); // id of the particle it is attached to
                                    get<chain_type>(particles)[particle_id(j)] = get<chain_type>(*k); // chain type is
                                    // the same as the one of the particle it is attached to
                                    //}

                                    //}

                                }


                            }

                        }
                    }

                    // if it is in the chain
                    if (get<chain>(particles[particle_id(j)]) > 0){
                        //try to move in the same direction as the cell it is attached to
                        vdouble2 x_chain = x + increase_fol_speed * get<direction>(particles)[particle_id(j)];

                        // Non-uniform domain growth
                        double x_in_chain;

                        x_in_chain =
                                (length_x / domain_length) * x_chain[0];//uniform growth in the first part of the domain



                        bool free_position = true;


                        // check if the position it wants to move is free
                        for (auto pos = euclidean_search(particles.get_query(), x_chain, diameter); pos != false; ++pos) {

                            if (get<id>(*pos) !=
                                get<id>(particles[particle_id(j)])) { // check if it is not the same particle
                                free_position = false;
                            }
                        }


                        // if the position is free and not out of bounds, move that direction
                        // full boundary
                        if (boundary == true){
                            if (free_position &&
                                (x_in_chain) > 0 &&
                                (x_in_chain) < length_x - 1 && (x_chain[1]) > 0 &&
                                (x_chain[1]) < length_y - 1) {
                                //cout << "direction " << get<direction>(particles[particle_id(j)]) << endl;
                                get<position>(particles)[particle_id(j)] +=
                                        increase_fol_speed * get<direction>(particles[particle_id(j)]);

                            }
                        }
                        //short boundary
                        if (boundary == false){
                            if(x_in_chain < boundlen){
                                if (free_position &&
                                    (x_in_chain) > 0 &&
                                    (x_in_chain) < length_x - 1 && (x_chain[1]) > 0 &&
                                    (x_chain[1]) < length_y - 1) {
                                    //cout << "direction " << get<direction>(particles[particle_id(j)]) << endl;
                                    get<position>(particles)[particle_id(j)] +=
                                            increase_fol_speed * get<direction>(particles[particle_id(j)]);

                                }
                            }else{
                                if (free_position &&
                                    (x_in_chain) > 0 &&
                                    (x_in_chain) < length_x - 1 ) {
                                    //cout << "direction " << get<direction>(particles[particle_id(j)]) << endl;
                                    get<position>(particles)[particle_id(j)] +=
                                            increase_fol_speed * get<direction>(particles[particle_id(j)]);

                                }
                            }

                        }


                    }


                    // if it hasn't found anything close, try to find a track or move randomly

                    if (get<chain>(particles[particle_id(j)]) == 0) {

                        //cout << "x before " << x << endl;
                        x = get<position>(particles[particle_id(j)]);
                        //cout << "x after " << x << endl;

                        //cout << "just before the found track " << endl;


                        /*
                         * Check if there is a track nearby
                         * */

                        // check what the closest neighbour is

                        vdouble2 diff; // difference in position
                        double previous_diff = dist_thres; // initialise distance threshold

                        /*for all the particles that are still not part of the chain
                        try to find a track of a leader cell to which it is close to
                         but make sure I do not
                         */
                        for (int k = 0; k < N; k++) {
                            for (int i = 0; i < track_length; i++) {
                                diff = x - track_position[i][k];
                                if (diff.norm() < dist_thres) {
                                    //cout << "previous diff " << previous_diff << endl;
                                    if (diff.norm() < previous_diff) {
                                        for (int f = 0;
                                             f <
                                             particles.size(); f++) { // make sure other particles are not in that position

                                            //cout << " time step value " << get<attached_at_time_step>(particles[f]) << endl;
                                            //cout << " attached to a leader " << get<attached_leader_nr>(particles[f])

                                            //cout << "value of j " << j << endl;
                                            //cout << "value of k " << k << endl;
                                            if (get<attached_at_time_step>(particles[f]) != j &&
                                                get<attached_leader_nr>(particles[f]) != k) {
                                                // have to make sure that it is not the same position as the leader
                                                //for (int p = 0; p < particles.size(); p++) {

                                                // this was in order to avoid follower cells moving on top of leader cells
                                                /* if ((track_position[j][k] - get<position>(particles[p])).norm() < 1) {
                                                     get<chain>(particles[i]) = 0;
                                                     break;
                                                 }*/
                                                //cout << "is it ever here?????? " << endl;
                                                get<attached_at_time_step>(particles[particle_id(j)]) = i;
                                                get<attached_leader_nr>(particles[particle_id(j)]) = k;

                                            }
                                        }
                                    }
                                }//else { get<chain>(particles[i]) = 0; }
                            }
                        }

                        // move in the direction where track goes

                        vdouble2 direc_follow = track_position[get<attached_at_time_step>(
                                particles[particle_id(j)])+1][get<attached_leader_nr>(
                                particles[particle_id(j)])] - track_position[get<attached_at_time_step>(
                                particles[particle_id(j)])][get<attached_leader_nr>(
                                particles[particle_id(j)])];

                        double normalise = direc_follow.norm();
                        //normalise
                        direc_follow = direc_follow/normalise;
                        vdouble2 x_can = x + direc_follow * speed_f;

                        // follow the track exactly
//                        vdouble2 direc_follow =  track_position[get<attached_at_time_step>(
//                                particles[particle_id(j)]) + 1][get<attached_leader_nr>(
//                                particles[particle_id(j)])]; //position where the cell wants to move
//
//                        vdouble2 x_can = direc_follow;




                        // check if that position is free
                        //cout << "Position "<< x << endl;

                        bool free_position = true; // check if the neighbouring position is free

                        // if this loop is entered, it means that there is another cell where I want to move
                        for (auto k = euclidean_search(particles.get_query(), x_can, diameter); k != false; ++k) {

                            //for (int i=0; i < particles.size(); i++) {
                            if (get<id>(*k) !=
                                get<id>(particles[particle_id(j)])) { // check if it is not the same particle
                                //cout << "reject step " << 1 << endl;
                                free_position = false;
                                //break;
                                //cout << "how frequently free position is false" << endl;
                            }
                        }



                        // it is possible that randomly they get dettached


                        // if the cell is part of the chain update its position
                        // full boundary
                        if (boundary == true){
                            if (free_position &&
                                ((x_can[0] * (length_x / domain_length))) > 0 &&
                                ((x_can[0] * (length_x / domain_length))) < length_x - 1 && (x_can[1]) > 0 &&
                                (x_can[1]) < length_y - 1) {

                                get<position>(particles[particle_id(j)]) = x_can;
                                get<attached_at_time_step>(particles[particle_id(j)]) += 1;
                                get<in_track>(particles[particle_id(j)]) = 1;
                            }
                            else{
                                get<in_track>(particles[particle_id(j)]) = 0;
                            }
                        }
                        // short boundary
                        if (boundary == false){
                            if(x_can[0] * (length_x / domain_length) < boundlen){
                                if (free_position &&
                                    ((x_can[0] * (length_x / domain_length))) > 0 &&
                                    ((x_can[0] * (length_x / domain_length))) < length_x - 1 && (x_can[1]) > 0 &&
                                    (x_can[1]) < length_y - 1) {

                                    get<position>(particles[particle_id(j)]) = x_can;
                                    get<attached_at_time_step>(particles[particle_id(j)]) += 1;
                                    get<in_track>(particles[particle_id(j)]) = 1;
                                }
                                else{
                                    get<in_track>(particles[particle_id(j)]) = 0;
                                }

                            }else{
                                if (free_position &&
                                   ((x_can[0] * (length_x / domain_length))) > 0 &&
                                   ((x_can[0] * (length_x / domain_length))) < length_x - 1 ) {

                                    get<position>(particles[particle_id(j)]) = x_can;
                                    get<attached_at_time_step>(particles[particle_id(j)]) += 1;
                                    get<in_track>(particles[particle_id(j)]) = 1;
                                }
                                else{
                                    get<in_track>(particles[particle_id(j)]) = 0;
                                }


                            }

                        }



                        // if it hasn't found a track
                        if (get<in_track>(particles[particle_id(j)]) == 0) {

                            x = get<position>(particles)[particle_id(j)]; // make sure that x is set correctly


                            double random_angle = uniformpi(gen1);


                            // full boundary
                            if (boundary == true){
                                while (((x_in + sin(random_angle) * l_filo_y) < 0 ||
                                        ((x_in + sin(random_angle) * l_filo_y)) >
                                        length_x - 1 || (x[1] + cos(random_angle) * l_filo_y) < 0 ||
                                        (x[1] + cos(random_angle) * l_filo_y) > length_y - 1)) {
                                    random_angle = uniformpi(gen1);
                                }
                            }
                            if (boundary == false){

                                random_angle = uniformpi(gen1);

                            }

                            x += speed_f * vdouble2(sin(random_angle), cos(random_angle));
                            // Non-uniform domain growth


                            // Non-uniform domain growth, onl first half grows
                            // if in the first part of the domain
                            x_in = (length_x / domain_length) * x[0];//uniform growth in the first part of the domain


                            bool free_position = true; // check if the neighbouring position is free

                            // if this loop is entered, it means that there is another cell where I want to mov

                            for (auto k = euclidean_search(particles.get_query(), x, diameter); k != false; ++k) {



                                //for (int i=0; i < particles.size(); i++) {
                                if (get<id>(*k) !=
                                    get<id>(particles[particle_id(j)])) { // check if it is not the same particle
                                    //cout << "reject step " << 1 << endl;
                                    free_position = false;
                                    break;
                                }
                            }



                            // check that the position they want to move to is free and not out of bounds

                            // full boundary
                            if (boundary == true){
                                if (free_position && (x_in) > 0 &&
                                    (x_in) < length_x - 1 && (x[1]) > 0 &&
                                    (x[1]) < length_y - 1) {
                                    //cout << " moves " << endl;
                                    //cout << "how frequently come in here " << endl;
                                    get<position>(particles)[particle_id(j)] += speed_f * vdouble2(sin(random_angle),
                                                                                                   cos(random_angle)); // update if nothing is in the next position
                                }
                            }
                            if (boundary == false){
                                if (x_in < boundlen){
                                    if (free_position && (x_in) > 0 &&
                                        (x_in) < length_x - 1 && (x[1]) > 0 &&
                                        (x[1]) < length_y - 1) {
                                        //cout << " moves " << endl;
                                        //cout << "how frequently come in here " << endl;
                                        get<position>(particles)[particle_id(j)] += speed_f * vdouble2(sin(random_angle),
                                                                                                       cos(random_angle)); // update if nothing is in the next position
                                    }
                                }else{
                                    if (free_position && (x_in) > 0 &&
                                        (x_in) < length_x - 1) {
                                        //cout << " moves " << endl;
                                        //cout << "how frequently come in here " << endl;
                                        get<position>(particles)[particle_id(j)] += speed_f * vdouble2(sin(random_angle),
                                                                                                       cos(random_angle)); // update if nothing is in the next position
                                    }
                                }

                            }





                        }
                    }


                }

                /* CHECK IF A FOLLOWER DOES NOT BECOME A LEADER
                * Alternative phenotypic switching if a follower overtakes a leader it becomes a leader and that leader follower.
                * I will have to be careful when there will be channels because I will have to choose the closest leader
                * */


                // find the closest leader


                // so that I would not go through all the cells I will choose the ones that are closer to the front

                // minimum position in x of the leaders

                int min_index = 0;

                for (int i = 1; i < N; ++i) {
                    if (get<position>(particles[i])[0] < get<position>(particles[min_index])[0]) {
                        min_index = i;
                    }

                }

                // if a follower is eps further in front than the leader, swap their types
                if (get<position>(particles[particle_id(j)])[0] > get<position>(particles[min_index])[0] + eps) {
                    // find distance to all the leaders
                    double distances[N];
                    vdouble2 dist_vector;
                    //check which one is the closest
                    for (int i = 0; i < N; ++i) {
                        dist_vector = get<position>(particles[particle_id(j)]) - get<position>(particles[i]);
                        distances[i] = dist_vector.norm();

                        int winning_index = 0;
                        for (int i = 1; i < N; ++i) {
                            if (distances[i] < distances[winning_index]) {
                                winning_index = i;
                            }
                        }

                        // if this closest leader is behind that follower, swap them
                        if (get<position>(particles[particle_id(j)])[0] >
                            get<position>(particles[winning_index])[0] + eps){//} && get<position>(particles[particle_id(j)])[1] < length_y -cell_radius && get<position>(particles[particle_id(j)])[1] > cell_radius) {
                            particle_type::value_type tmp = particles[winning_index];


                            // their position swap

                            vdouble2 temp = get<position>(particles[winning_index]);
                            get<position>(particles[winning_index]) = get<position>(particles[particle_id(j)]);
                            get<position>(particles[particle_id(j)]) = temp;


                        }

                    }


                }

            }

        }
        for(int i=0; i<N; i++) {
            //if (get<type>(particles[i]) == 0){
            vdouble2 diff;

            // if shorter than track length
            if (track_time[i] < track_length) {
                diff = track_position[track_time[i]][i] - get<position>(particles[i]); // the difference is some intermediate vecotr value

            }// if longer than track length
            else {
                diff = track_position[track_length-1][i] - get<position>(particles[i]); // the last position
            }
            //cout << "get position " << get<position>(particles[i]) << endl;
            if (diff.norm() > track_spacing) {
                track_time[i] += 1; // update time steps

                if (track_time[i] < track_length) {

                    track_position[track_time[i]][i] = get<position>(particles[i]);
                }// if longer than track length
                if (track_time[i] > track_length) {
                    for (int j = 0; j < track_length; j++) {
                        track_position[j][i] = track_position[j + 1][i];// vector shifts by one
                    }
                    track_position[track_length-1][i] = get<position>(particles[i]); // new position added

                }
            }
            //cout << "track time " << track_time[i]<< endl;
            //}

        }

        // update positions
        particles.update_positions();


        // save at every time step
        if (t % 100 ==0){
#ifdef HAVE_VTK
            vtkWriteGrid("M11cells", t, particles.get_grid(true));
#endif
        }

// save data to track positions
        ofstream output1("track_positions.csv");

        output1 << "x, y, z, u" << "\n" << endl;

        vdouble2 posi;

        for (int i=0;i<track_length;i++){
            for(int j=0;j<2;j++){
                if(j==1){
                    output1 << 0 << ", ";
                }
                else{
                    posi = track_position[i][j];
                    output1 << posi[0] << ", " << posi[1] << ", ";

                }
            }
            output1 << "\n" << endl;
        }

    }

    /*
     * return the density of cells in domain_partition parts of the domain
     */
    const int domain_partition = int(domain_length / double(5));; // number of intervalas of 50 \mu m

    VectorXi proportions = VectorXi::Zero(domain_partition); // integer with number of cells in particular part

    double one_part = domain_length / double(domain_partition);


    for (int i = 0; i < domain_partition; i++) {

        for (int j = 0; j < particles.size(); j++) {
            vdouble2 x = get<position>(particles[j]);
            if (i * one_part < x[0] && x[0] < (i + 1) * one_part) {
                proportions(i) += 1;
            }
        }

    }

    return proportions;


}



/*
 * main for proportions in different sections
 */


// parameter analysis
int main(){

    const int number_parameters = 1; // parameter range
    const int sim_num = 1;

    //VectorXi vector_check_length = proportions(0.05, 2); //just to know what the length is
    //cout << "prop " << vector_check_length << endl;
    //int num_parts = vector_check_length.size(); // number of parts that I partition my domain
    int num_parts = 21; // for 1800 timesteps
    MatrixXf sum_of_all = MatrixXf::Zero(num_parts,number_parameters); // sum of the values over all simulations

    // n would correspond to different seeds
    // parallel programming
#pragma omp parallel for
    for (int n = 0; n < sim_num; n++) {

        // define parameters that I will change

        array<double, number_parameters> threshold;
        array<double, 1> slope;

        // set the parameters
        for (int i = 0; i < number_parameters; i++) {
            threshold[i] = 0.1;
            //threshold[i] = 0.005 * (i + 1);// 0.01;
            //cout << "slope " << slope[i] << endl;

        }

        //initialise the matrix to store the values
        MatrixXi numbers = MatrixXi::Zero(num_parts,number_parameters);

        //#pragma omp parallel for
        //        for (int i = 0; i < number_parameters; i++) {

        //for (int j = 0; j < 1; j++) {

        numbers.block(0,0,num_parts,1) = proportions(threshold[0], 0);

        //}
        // }


//        // This is what I am using for MATLAB
//        ofstream output2("control_case.csv");
//
//        for (int i = 0; i < numbers.rows(); i++) {
//
//            for (int j = 0; j < numbers.cols(); j++) {
//
//                output2 << numbers(i, j) << ", ";
//
//                sum_of_all(i,j) += numbers(i,j);
//
//            }
//            output2 << "\n" << endl;
//        }

    }
    /*
    * will store everything in one matrix, the entries will be summed over all simulations
    */

//    ofstream output3("aqp_M9.csv");
//
//    for (int i = 0; i < num_parts; i++) {
//
//        for (int j = 0; j < number_parameters; j++) {
//
//            output3 << sum_of_all(i, j) << ", ";
//
//        }
//        output3 << "\n" << endl;
//    }


}

