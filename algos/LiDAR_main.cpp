//
//  LiDAR_main.cpp
//  Gravity
//
//  Created by Hassan Hijazi on 3 April 2020.
//
//
#include <stdio.h>
#include <gravity/solver.h>
#ifdef USE_OPT_PARSER
#include <optionParser.hpp>
#endif
#include <gravity/rapidcsv.h>
//#undefine USE_MATPLOT
#ifdef USE_MATPLOT
#include <gravity/matplotlibcpp.h>
#endif
#include <queue>
#include "DataSet.h"
#include "lasreader.hpp"
#include "laswriter.hpp"
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <future>
#include <thread>
#include "Lidar_utils.h"
#include <math.h>
#include <gravity/solver.h>
#include "ARMOConfig.h"
#include "BB.h"
#include "Lower_Bound.h"

#ifdef USE_EIGEN3
#include <Eigen/Dense>
#endif
#include <time.h>

#ifdef USE_GJK
extern "C" {
#include "openGJK.h"
}
#endif
#include <iostream>
#include <algorithm>
using namespace std;
using namespace gravity;


/* Plot two point clouds */
#ifdef USE_MATPLOT
//void plot(const vector<vector<double>>& ext_model, const vector<vector<double>>& ext_data, double point_thick=0.1);
/* Plot two point clouds */
//void plot(const vector<vector<double>>& ext_model, const vector<vector<double>>& ext_data, double point_thick=0.1){
//    namespace plt = matplotlibcpp;
//    vector<double> x_vec_model(ext_model.size()), y_vec_model(ext_model.size()), z_vec_model(ext_model.size());
//    vector<double> x_vec_data(ext_data.size()), y_vec_data(ext_data.size()), z_vec_data(ext_data.size());
//    for (int i = 0; i<ext_model.size(); i++) {
//        x_vec_model[i] = ext_model[i][0];
//        y_vec_model[i] = ext_model[i][1];
//        z_vec_model[i] = ext_model[i][2];
//    }
//    for (int i = 0; i<ext_data.size(); i++) {
//        x_vec_data[i] = ext_data[i][0];
//        y_vec_data[i] = ext_data[i][1];
//        z_vec_data[i] = ext_data[i][2];
//    }
//    std::map<std::string, std::string> keywords;
//    keywords["marker"] = "s";
//    keywords["linestyle"] = "None";
//    keywords["ms"] = to_string(point_thick);
//    plt::plot3(x_vec_model, y_vec_model, z_vec_model,x_vec_data, y_vec_data, z_vec_data, keywords);
//    plt::show();
//}
#endif
//void plot(const vector<vector<double>>& point_cloud1, const vector<vector<double>>& point_cloud2, double point_thick = 0.1);
//void plot(const vector<vector<double>>& point_cloud1, const vector<vector<double>>& point_cloud2,const vector<vector<double>>& point_cloud3, double point_thick = 0.1);
//void plot(const vector<vector<double>>& ext_model, const vector<vector<double>>& ext_data, const vector<vector<double>>& ext_data1,const vector<vector<double>>& ext_data2, double point_thick=0.1);

int main (int argc, char * argv[])
{
    DebugOn("\n********************************************************************************\n");
    DebugOn("********************************  ARMO  ****************************************\n");
    DebugOn("********************************************************************************\n");
    DebugOn("This program uses Gravity and LasLib and is released as open-source code under\n");
    DebugOn("the BSD-3 License. For more information visit https://github.com/lanl-ansi/ARMO\n");
    DebugOn("********************************************************************************\n");
    DebugOn("********************************************************************************\n");
    DebugOn("********************************************************************************\n\n");
#ifdef USE_MPI
    auto err_init = MPI_Init(nullptr,nullptr);
    int worker_id, nb_workers;
    auto err_rank = MPI_Comm_rank(MPI_COMM_WORLD, &worker_id);
    auto err_size = MPI_Comm_size(MPI_COMM_WORLD, &nb_workers);
#endif
    string file= string(armo_dir)+"/datasets/Truck.adc.laz";
    /*Scanner offset*/
    double scanner_x=0.0, scanner_y=0.160999998450279, scanner_z=0.016;
    double hr=0,hp=0,hy=0;
    double max_time = 100;
    int max_nb_points = 5000;
    double bore_roll=0, bore_pitch=0, bore_yaw=0;/*Calibration angles in degrees*/
    string algo="aGS";
    bool data_opt=true;/*If true, Working with data set \hat{P} union \bar{P}, else data set \hat{D} union \bar{D}  */
    bool format_laz=true;
    /*Algorithm Choices
     "aGS"(default) to run the heuristic upper bound
     "nsBB" to run the spatial branch and bound algorithm
     "gurobi" to run gurobi on the problem
     else if 3 angles given, the data-set is calibrated with these
     */
    if(argc>=2){
        file = argv[1];
    }
    if(argc>=3){
        algo = argv[2];
    }
    if(argc==4){
        max_time = std::stod(argv[3]);
    }
    if(argc>=5){
        max_nb_points = std::stoi(argv[4]);
    }
    if(argc>=6){
        algo="apply_angles";
        bore_roll= std::stod(argv[2]);
        bore_pitch= std::stod(argv[3]);
        bore_yaw= std::stod(argv[4]);
        data_opt=false;
    }
    if(argc>=7){
        format_laz=false;
    }
    
 
    string error_type="L2";
    if(algo=="aGSL1"){
        error_type="L1";
    }
    vector<double> best_rot(9,0.0);
    
    double best_ub=1e5,L2init, L1init;
    vector<vector<double>> lidar_point_cloud, roll_pitch_yaw, em;
    vector<vector<double>> full_point_cloud_model, full_point_cloud_data, full_uav_model, full_uav_data,full_rpy_model, full_rpy_data;
    vector<vector<double>> point_cloud_model, point_cloud_data,rpy_model, rpy_data,uav_model, uav_data;;
   
     DebugOn("Algorithm "<<algo<<" chosen"<<endl);
    /*Reads input laz*/
    auto uav_cloud=::read_laz(file, lidar_point_cloud, roll_pitch_yaw);
    
    if(data_opt){/*Working with data sets P*/
//        plot(uav_cloud, uav_cloud,0.1);
        flight_lines_split(lidar_point_cloud,uav_cloud, roll_pitch_yaw, full_point_cloud_model,  full_uav_model,full_rpy_model,full_point_cloud_data, full_uav_data,full_rpy_data);
        
        subsample_overlap_scale(file, full_point_cloud_model, full_uav_model, full_rpy_model, full_point_cloud_data, full_uav_data, full_rpy_data, uav_cloud.at(0), point_cloud_model,  uav_model, rpy_model, point_cloud_data, uav_data, rpy_data,hr,hp,hy,max_nb_points);
        DebugOn("Size of set bar P "<<point_cloud_model.size()<<endl);
        DebugOn("Size of set hat P "<<point_cloud_data.size()<<endl);
        
        double roll_min=-2*pi/180;
        double roll_max=2*pi/180;
        double pitch_min=-2*pi/180;
        double pitch_max=2*pi/180;
        double yaw_min=-2*pi/180;
        double yaw_max=2*pi/180;
        
        auto point_cloud_model_copy=point_cloud_model;
        auto point_cloud_data_copy=point_cloud_data;
        
        vector<int> matching(point_cloud_data.size());
        vector<double> err_per_point(point_cloud_data.size());
        
        L2init=computeL2error(point_cloud_model_copy,point_cloud_data_copy,matching,err_per_point);
        L1init=computeL1error(point_cloud_model_copy,point_cloud_data_copy,matching,err_per_point);
#ifdef USE_MPI
        if(worker_id==0){
#endif
            DebugOn("\n*************************\n");
            DebugOn("L2 Initial  "<<L2init<<endl);
            DebugOn("L1 Initial  "<<L1init<<endl);
            DebugOn("*************************\n");
#ifdef USE_MPI
        }
#endif
        
        if(error_type=="L2"){
            best_ub=L2init;
        }
        else{
            best_ub=L1init;
        }
        if(algo=="aGS" || algo=="aGSL1"){
#ifdef USE_MATPLOT
            plot(uav_model, uav_data,0.1);
            plot(point_cloud_model, point_cloud_data,0.1);
#endif
	    DebugOn("Running "<<algo<<" ..."<<endl);
            auto rot= ub_heuristic_disc(point_cloud_model, point_cloud_data, uav_model, uav_data, rpy_model, rpy_data, best_rot, best_ub, error_type, scanner_x, scanner_y, scanner_z, hr, hp, hy, max_time);
            auto roll_rad_ub = rot[0];
            auto pitch_rad_ub = rot[1];
            auto yaw_rad_ub = rot[2];
            DebugOff("Angles radians "<<roll_rad_ub<<" "<<pitch_rad_ub<<" "<<yaw_rad_ub<<endl);
            DebugOn("Angles (degrees) "<<roll_rad_ub*180/pi<<" "<<pitch_rad_ub*180/pi<<" "<<yaw_rad_ub*180/pi<<endl);
            apply_transform_new_order(roll_rad_ub, pitch_rad_ub, yaw_rad_ub, lidar_point_cloud, uav_cloud, roll_pitch_yaw, scanner_x,scanner_y,scanner_z,hr,hp,hy);
            save_laz(file.substr(0,file.find(".laz"))+"_"+to_string(roll_rad_ub*180/pi)+"_"+to_string(pitch_rad_ub*180/pi)+"_"+to_string(yaw_rad_ub*180/pi)+".laz", lidar_point_cloud, em);
            
        }
        else if(algo=="gurobi"){
	    DebugOn("Running "<<algo<<" ..."<<endl);
            vector<vector<double>> input_data_cloud, input_model_cloud, input_data_offset, input_model_offset;
            generate_inputs(point_cloud_model, uav_model, rpy_model, scanner_x, scanner_y, scanner_z,hr,hp,hy, input_model_cloud, input_model_offset);
            generate_inputs(point_cloud_data, uav_data, rpy_data, scanner_x, scanner_y, scanner_z, hr,hp,hy,input_data_cloud, input_data_offset);
            auto rot_h= ub_heuristic_disc(point_cloud_model, point_cloud_data, uav_model, uav_data, rpy_model, rpy_data, best_rot, best_ub, error_type, scanner_x, scanner_y, scanner_z, hr, hp, hy);
            indices N1 = range(1,point_cloud_data.size());
            indices N2 = range(1,point_cloud_model.size());
            indices new_cells(N1,N2);
            param<double> dist_cells("dist_cells");
            auto model_i=Align_L2_model_rotation_trigonometric_scanner(input_model_cloud, input_data_cloud, uav_model, uav_data, rpy_model, rpy_data, input_model_offset, input_data_offset, roll_min, roll_max, pitch_min, pitch_max, yaw_min ,yaw_max, new_cells, dist_cells);
            int num_thread=thread::hardware_concurrency();
#ifdef USE_GUROBI
            solver<> S1(model_i,gurobi);
            S1.run(5,1e-6,"",9000000,10000000, best_ub, num_thread);
#endif
            
        }
        else if(algo=="nsBB"){/*Run the nsBB algorithm*/
            auto rot_h= ub_heuristic_disc(point_cloud_model, point_cloud_data, uav_model, uav_data, rpy_model, rpy_data, best_rot, best_ub, error_type, scanner_x, scanner_y, scanner_z, hr, hp, hy);
            vector<double> rot;
#ifdef USE_GJK
#ifdef USE_MPI
	    DebugOn("Running "<<algo<<" ..."<<endl); 
            rot=BranchBound_MPI(point_cloud_model, point_cloud_data, uav_model, uav_data, rpy_model, rpy_data, rot_h, best_ub, error_type, scanner_x, scanner_y, scanner_z, hr, hp, hy);
#else
	    DebugOn("Running "<<algo<<" ..."<<endl); 
            rot= BranchBound_Align(point_cloud_model, point_cloud_data, uav_model, uav_data, rpy_model, rpy_data, rot_h, best_ub, error_type, scanner_x, scanner_y, scanner_z, hr, hp, hy);
#endif
            
            auto roll_deg_bb = rot[0];
            auto pitch_deg_bb = rot[1];
            auto yaw_deg_bb = rot[2];
            
            apply_transform_new_order(roll_deg_bb*pi/180, pitch_deg_bb*pi/180, yaw_deg_bb*pi/180, point_cloud_model, uav_model, rpy_model, scanner_x,scanner_y,scanner_z,hr,hp,hy);
            apply_transform_new_order(roll_deg_bb*pi/180, pitch_deg_bb*pi/180, yaw_deg_bb*pi/180, point_cloud_data, uav_data, rpy_data, scanner_x,scanner_y,scanner_z,hr,hp,hy);
            save_laz(file.substr(0,file.find(".laz"))+to_string(roll_deg_bb)+"_"+to_string(pitch_deg_bb)+"_"+to_string(yaw_deg_bb)+"_hatp.laz", point_cloud_data, em);
            save_laz(file.substr(0,file.find(".laz"))+to_string(roll_deg_bb)+"_"+to_string(pitch_deg_bb)+"_"+to_string(yaw_deg_bb)+"_barp.laz", point_cloud_model, em);
            apply_transform_new_order(roll_deg_bb*pi/180, pitch_deg_bb*pi/180, yaw_deg_bb*pi/180, lidar_point_cloud, uav_cloud, roll_pitch_yaw, scanner_x,scanner_y,scanner_z,hr,hp,hy);
            save_laz(file.substr(0,file.find(".laz"))+"_"+to_string(roll_deg_bb)+"_"+to_string(pitch_deg_bb)+"_"+to_string(yaw_deg_bb)+".laz", lidar_point_cloud, em);
            
            auto L2=computeL2error(point_cloud_model,point_cloud_data,matching,err_per_point);
            auto L1=computeL1error(point_cloud_model,point_cloud_data,matching,err_per_point);
            
#ifdef USE_MPI
            if(worker_id==0){
#endif
                
                DebugOn("L2  "<<L2<<endl);
                DebugOn("L1  "<<L1<<endl);
                
                DebugOn("Percentage improved L2 "<<(L2init-L2)/L2init*100.0<<endl);
                DebugOn("Percentage improved L1 "<<(L1init-L1)/L1init*100.0<<endl);
#ifdef USE_MPI
            }
#endif
#endif
        }
        else{/*Apply the calibration values on sets \hat{P}, \bar{P} and \hat{P} union \bar{P} */
            apply_transform_new_order(bore_roll*pi/180, bore_pitch*pi/180, bore_yaw*pi/180, point_cloud_model, uav_model, rpy_model, scanner_x,scanner_y,scanner_z,hr,hp,hy);
            apply_transform_new_order(bore_roll*pi/180, bore_pitch*pi/180, bore_yaw*pi/180, point_cloud_data, uav_data, rpy_data, scanner_x,scanner_y,scanner_z,hr,hp,hy);
            apply_transform_new_order(bore_roll*pi/180, bore_pitch*pi/180, bore_yaw*pi/180, lidar_point_cloud, uav_cloud, roll_pitch_yaw, scanner_x, scanner_y, scanner_z, hr, hp, hy);
            save_laz(file.substr(0,file.find(".laz"))+to_string(bore_roll)+"_"+to_string(bore_pitch)+"_"+to_string(bore_yaw)+"_hatp.laz", point_cloud_data, em);
            save_laz(file.substr(0,file.find(".laz"))+to_string(bore_roll)+"_"+to_string(bore_pitch)+"_"+to_string(bore_yaw)+"_barp.laz", point_cloud_model, em);
            save_laz(file.substr(0,file.find(".laz"))+to_string(bore_roll)+"_"+to_string(bore_pitch)+"_"+to_string(bore_yaw)+".laz", lidar_point_cloud, em);
            auto L2=computeL2error(point_cloud_model,point_cloud_data,matching,err_per_point);
            auto L1=computeL1error(point_cloud_model,point_cloud_data,matching,err_per_point);
#ifdef USE_MPI
            if(worker_id==0){
#endif
                DebugOn("L2  "<<L2<<endl);
                DebugOn("L1  "<<L1<<endl);
                
                DebugOn("Percentage improved L2 "<<(L2init-L2)/L2init*100.0<<endl);
                DebugOn("Percentage improved L1 "<<(L1init-L1)/L1init*100.0<<endl);
#ifdef USE_MPI
            }
#endif
        }
    }
    else{/*Apply the calibration values on large data set D=\hat{D} union \bar{D}*/
        
        apply_transform_new_order(bore_roll*pi/180, bore_pitch*pi/180, bore_yaw*pi/180, lidar_point_cloud, uav_cloud, roll_pitch_yaw, scanner_x, scanner_y, scanner_z, hr, hp, hy);
        if(format_laz){
            save_laz(file.substr(0,file.find(".laz"))+to_string(bore_roll)+"_"+to_string(bore_pitch)+"_"+to_string(bore_yaw)+".laz", lidar_point_cloud, em);
        }
        else{
            save_laz(file.substr(0,file.find(".laz"))+to_string(bore_roll)+"_"+to_string(bore_pitch)+"_"+to_string(bore_yaw)+".las", lidar_point_cloud, em);
        }
    }
//#ifdef USE_MATPLOT
//    plot(point_cloud_model, point_cloud_data);
//#endif
    
}

