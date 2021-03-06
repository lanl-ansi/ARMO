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
void plot(const vector<vector<double>>& point_cloud1, const vector<vector<double>>& point_cloud2, double point_thick = 0.1);
void plot(const vector<vector<double>>& point_cloud1, const vector<vector<double>>& point_cloud2,const vector<vector<double>>& point_cloud3, double point_thick = 0.1);
void plot(const vector<vector<double>>& ext_model, const vector<vector<double>>& ext_data, const vector<vector<double>>& ext_data1,const vector<vector<double>>& ext_data2, double point_thick=0.1);

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
    string file_u= string(armo_dir)+"/datasets/Truck.adc.laz";
    /*Scanner offset*/
    double scanner_x=0.0, scanner_y=0.160999998450279, scanner_z=0.016;
    /*"hidden" calibration applied by LiDAR viewer given in .json file*/
    /*Truck set*/
    double hr=0, hp=0, hy=0;
    /*to select overlapping regions of the object*/
    /*Truck set*/
    double xm=0, ym=0,zm=0,xd=0,yd=0,zd=0;
    hr=0,hp=0,hy=0;
    /*to downsample points*/
    /*Truck set*/
    int mskip =1, dskip =1;
    double max_time = 100;
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
        file_u = argv[1];
    }
    if(argc>=3){
        algo = argv[2];
    }
    if(argc==4){
        max_time = std::stod(argv[3]);
    }
    if(argc>=5){
        algo="apply_angles";
        bore_roll= std::stod(argv[2]);
        bore_pitch= std::stod(argv[3]);
        bore_yaw= std::stod(argv[4]);
        data_opt=false;
    }
    if(argc>=6){
        format_laz=false;
    }
    if(file_u.find("Truck.adc.laz")!=std::string::npos){
        DebugOn("Truck data set selected"<<endl);
    }
    else if(file_u.find("Car.adc.laz")!=std::string::npos){
        hr=0;hp=0;hy=0;
        xm=0, ym=0,zm=2122.0,xd=385276,yd=0,zd=2121.4;
        mskip=1,dskip=4;
        DebugOn("Car data set selected"<<endl);
    }
    else if(file_u.find("Tent.adc.laz")!=std::string::npos){
        hr=0;hp=0;hy=0;
        xm=0; ym=0;zm=124;xd=0;yd=0;zd=124.2;
        mskip=2;dskip=3;
        DebugOn("Tent data set selected"<<endl);
    }
    else{
        DebugOn("WARNING: New data set!!!"<<endl<<"Before continuing "<<endl);
        DebugOn("Check if values of hidden calibration values are updated"<<endl);
        DebugOn("Check if values of xm,ym,zm,xd,yd,zd are updated"<<endl);
        DebugOn("Check if values of mskip and dskip are updated"<<endl);
    }
    string error_type="L2";
    if(algo=="aGSL1"){
        error_type="L1";
    }
    vector<double> best_rot(9,0.0);
    
    double best_ub=1e5,L2init, L1init;
    
    vector<vector<double>> full_point_cloud_model, full_point_cloud_data, full_uav_model, full_uav_data;
    vector<vector<double>> point_cloud_model, point_cloud_data,point_cloud_model1, point_cloud_data1;
    vector<vector<double>> uav_model, uav_data,uav_model1, uav_data1,cloud1, cloud2, uav1, uav2, rpy1, rpy2;
    vector<vector<double>> full_rpy_model, full_rpy_data, rpy_model, rpy_data,rpy_model1, rpy_data1;
    vector<vector<double>> lidar_point_cloud, roll_pitch_yaw, em;
    /*Reads input laz*/
    auto uav_cloud_u=::read_laz(file_u, lidar_point_cloud, roll_pitch_yaw);
    
    if(data_opt){/*Working with data sets P*/
        int mid_i=0;
        /*Separating data in parallel flight lines 1 and 2*/
        for(auto i=1;i<uav_cloud_u.size();i++)
        {
            auto x=uav_cloud_u.at(i)[0];
            auto y=uav_cloud_u.at(i)[1];
            auto z=uav_cloud_u.at(i)[2];
            auto x_prev=uav_cloud_u.at(i-1)[0];
            auto y_prev=uav_cloud_u.at(i-1)[1];
            auto z_prev=uav_cloud_u.at(i-1)[2];
            if((abs(x-x_prev)>=1 && abs(y-y_prev)>=1)){
                if(mid_i!=0){
                    DebugOn("More than Two flight lines are detected "<<mid_i<<" "<<endl);
                    DebugOn("Invalid flight line selection!!!!!!!!!!!!!!!!!!!!!!!!");
                    exit(0);
                }
                mid_i=i;
                DebugOn("Two flight lines are detected "<<mid_i<<endl);
                
            }
        }
        /*If two flight lines identified2*/
        if(mid_i==0){
            invalid_argument("Two flight lines are not detected!");
        }
        /*For downsampling*/
        int cloud_bar_max=1e4;
        int cloud_hat_max=2e3;
        int size_1=mid_i;
        int size_2=lidar_point_cloud.size()-mid_i+1;
        int skip_1a=1, skip_1b=1, skip_2a=1, skip_2b=1;
        bool check_rem1=false, check_rem2=false;
        if(size_1>=size_2){
            if(size_1>cloud_bar_max){
                double diff=size_1-cloud_bar_max;
                if(diff<cloud_bar_max){
                    skip_1b=round(size_1/diff);
                    check_rem1=true;
                }
                else{
                    skip_1a=round(size_1/cloud_bar_max);
                }
            }
            if(size_2>cloud_hat_max){
                double diff=size_2-cloud_hat_max;
                if(diff<cloud_hat_max){
                    skip_2b=round(size_2/diff);
                    check_rem2=true;
                }
                else{
                    skip_2a=round(size_2/cloud_hat_max);
                }
            }
        }
        else{
            if(size_2>cloud_bar_max){
                double diff=size_2-cloud_bar_max;
                if(diff<cloud_bar_max){
                    skip_2b=round(size_2/diff);
                    check_rem2=true;
                }
                else{
                    skip_2a=round(size_2/cloud_bar_max);
                }
            }
            if(size_1>cloud_hat_max){
                double diff=size_1-cloud_hat_max;
                if(diff<cloud_hat_max){
                    skip_1b=round(size_1/diff);
                    check_rem1=true;
                }
                else{
                    skip_1a=round(size_1/cloud_hat_max);
                }
            }
        }
        for(auto i=0;i<mid_i;i+=skip_1a){
            if(check_rem1 && i%skip_1b==0){
                continue;
            }
            cloud1.push_back(lidar_point_cloud.at(i));
            uav1.push_back(uav_cloud_u.at(i));
            rpy1.push_back(roll_pitch_yaw.at(i));
        }
        for(auto i=mid_i;i<lidar_point_cloud.size();i+=skip_2a){
            if(check_rem2 && i%skip_2b==0){
                continue;
            }
            cloud2.push_back(lidar_point_cloud.at(i));
            uav2.push_back(uav_cloud_u.at(i));
            rpy2.push_back(roll_pitch_yaw.at(i));
        }
        DebugOff("cloud1.size() "<<cloud1.size()<<endl);
        DebugOff("cloud2.size() "<<cloud2.size()<<endl);
        if(cloud1.size()>=cloud2.size()){
            full_point_cloud_model=cloud1;
            full_point_cloud_data=cloud2;
            full_uav_model=uav1;
            full_uav_data=uav2;
            full_rpy_model=rpy1;
            full_rpy_data=rpy2;
        }
        else{
            full_point_cloud_model=cloud2;
            full_point_cloud_data=cloud1;
            full_uav_model=uav2;
            full_uav_data=uav1;
            full_rpy_model=rpy2;
            full_rpy_data=rpy1;
        }
#ifdef USE_MATPLOT
        plot(full_point_cloud_model, full_point_cloud_data);
#endif
        vector<vector<double>> e;
        e.push_back(full_point_cloud_model[0]);
#ifdef USE_MATPLOT
        plot(full_point_cloud_model,e);
#endif
        e.clear();
        e.push_back(full_point_cloud_data[0]);
#ifdef USE_MATPLOT
        plot(full_point_cloud_data,e);
#endif
        for(auto i=0;i<full_point_cloud_model.size();i++){
            auto x=full_point_cloud_model.at(i)[0];
            auto y=full_point_cloud_model.at(i)[1];
            auto z=full_point_cloud_model.at(i)[2];
            if(x>=xm && y>=ym && z>=zm){
                point_cloud_model1.push_back(full_point_cloud_model.at(i));
                uav_model1.push_back(full_uav_model.at(i));
                rpy_model1.push_back(full_rpy_model.at(i));
            }
        }
        for(auto i=0;i<full_point_cloud_data.size();i++){
            auto x=full_point_cloud_data.at(i)[0];
            auto y=full_point_cloud_data.at(i)[1];
            auto z=full_point_cloud_data.at(i)[2];
            if(x>=xd && y>=yd && z>=zd){
                point_cloud_data1.push_back(full_point_cloud_data.at(i));
                uav_data1.push_back(full_uav_data.at(i));
                rpy_data1.push_back(full_rpy_data.at(i));
            }
        }
        DebugOff(point_cloud_model1.size()<<endl);
        DebugOff(point_cloud_data1.size()<<endl);
#ifdef USE_MATPLOT
        plot(point_cloud_model1, point_cloud_data1);
        plot(uav_model1,uav_data1);
#endif
        
        for(auto i=0;i<point_cloud_model1.size();i+=mskip){
            point_cloud_model.push_back(point_cloud_model1.at(i));
            uav_model.push_back(uav_model1.at(i));
            rpy_model.push_back(rpy_model1.at(i));
            
        }
        
        for(auto i=0;i<point_cloud_data1.size();i+=dskip){
            point_cloud_data.push_back(point_cloud_data1.at(i));
            uav_data.push_back(uav_data1.at(i));
            rpy_data.push_back(rpy_data1.at(i));
            
        }
        save_laz(file_u.substr(0,file_u.find(".laz"))+"_model.laz", point_cloud_model, em);
        save_laz(file_u.substr(0,file_u.find(".laz"))+"_data.laz", point_cloud_data, em);
        bool scale=true;
        if(scale){
            for(auto i=0;i<point_cloud_model.size();i++){
                point_cloud_model.at(i)[0]-=uav_cloud_u.at(0)[0];
                point_cloud_model.at(i)[1]-=uav_cloud_u.at(0)[1];
                point_cloud_model.at(i)[2]-=uav_cloud_u.at(0)[2];
            }
            for(auto i=0;i<point_cloud_data.size();i++){
                point_cloud_data.at(i)[0]-=uav_cloud_u.at(0)[0];
                point_cloud_data.at(i)[1]-=uav_cloud_u.at(0)[1];
                point_cloud_data.at(i)[2]-=uav_cloud_u.at(0)[2];
            }
            for(auto i=0;i<uav_model.size();i++){
                uav_model.at(i)[0]-=uav_cloud_u.at(0)[0];
                uav_model.at(i)[1]-=uav_cloud_u.at(0)[1];
                uav_model.at(i)[2]-=uav_cloud_u.at(0)[2];
            }
            for(auto i=0;i<uav_data.size();i++){
                uav_data.at(i)[0]-=uav_cloud_u.at(0)[0];
                uav_data.at(i)[1]-=uav_cloud_u.at(0)[1];
                uav_data.at(i)[2]-=uav_cloud_u.at(0)[2];
            }
        }
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
            auto rot= ub_heuristic_disc(point_cloud_model, point_cloud_data, uav_model, uav_data, rpy_model, rpy_data, best_rot, best_ub, error_type, scanner_x, scanner_y, scanner_z, hr, hp, hy, max_time);
            auto roll_rad_ub = rot[0];
            auto pitch_rad_ub = rot[1];
            auto yaw_rad_ub = rot[2];
            DebugOff("Angles radians "<<roll_rad_ub<<" "<<pitch_rad_ub<<" "<<yaw_rad_ub<<endl);
            DebugOn("Angles (degrees) "<<roll_rad_ub*180/pi<<" "<<pitch_rad_ub*180/pi<<" "<<yaw_rad_ub*180/pi<<endl);
            apply_transform_new_order(roll_rad_ub, pitch_rad_ub, yaw_rad_ub, lidar_point_cloud, uav_cloud_u, roll_pitch_yaw, scanner_x,scanner_y,scanner_z,hr,hp,hy);
            save_laz(file_u.substr(0,file_u.find(".laz"))+"_"+to_string(roll_rad_ub*180/pi)+"_"+to_string(pitch_rad_ub*180/pi)+"_"+to_string(yaw_rad_ub*180/pi)+".laz", lidar_point_cloud, em);
            
        }
        else if(algo=="gurobi"){
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
            rot=BranchBound_MPI(point_cloud_model, point_cloud_data, uav_model, uav_data, rpy_model, rpy_data, rot_h, best_ub, error_type, scanner_x, scanner_y, scanner_z, hr, hp, hy);
#else
            rot= BranchBound_Align(point_cloud_model, point_cloud_data, uav_model, uav_data, rpy_model, rpy_data, rot_h, best_ub, error_type, scanner_x, scanner_y, scanner_z, hr, hp, hy);
#endif
            
            auto roll_deg_bb = rot[0];
            auto pitch_deg_bb = rot[1];
            auto yaw_deg_bb = rot[2];
            
            apply_transform_new_order(roll_deg_bb*pi/180, pitch_deg_bb*pi/180, yaw_deg_bb*pi/180, point_cloud_model, uav_model, rpy_model, scanner_x,scanner_y,scanner_z,hr,hp,hy);
            apply_transform_new_order(roll_deg_bb*pi/180, pitch_deg_bb*pi/180, yaw_deg_bb*pi/180, point_cloud_data, uav_data, rpy_data, scanner_x,scanner_y,scanner_z,hr,hp,hy);
            save_laz(file_u.substr(0,file_u.find(".laz"))+to_string(roll_deg_bb)+"_"+to_string(pitch_deg_bb)+"_"+to_string(yaw_deg_bb)+"_hatp.laz", point_cloud_data, em);
            save_laz(file_u.substr(0,file_u.find(".laz"))+to_string(roll_deg_bb)+"_"+to_string(pitch_deg_bb)+"_"+to_string(yaw_deg_bb)+"_barp.laz", point_cloud_model, em);
            apply_transform_new_order(roll_deg_bb*pi/180, pitch_deg_bb*pi/180, yaw_deg_bb*pi/180, lidar_point_cloud, uav_cloud_u, roll_pitch_yaw, scanner_x,scanner_y,scanner_z,hr,hp,hy);
            save_laz(file_u.substr(0,file_u.find(".laz"))+"_"+to_string(roll_deg_bb)+"_"+to_string(pitch_deg_bb)+"_"+to_string(yaw_deg_bb)+".laz", lidar_point_cloud, em);
            
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
            apply_transform_new_order(bore_roll*pi/180, bore_pitch*pi/180, bore_yaw*pi/180, lidar_point_cloud, uav_cloud_u, roll_pitch_yaw, scanner_x, scanner_y, scanner_z, hr, hp, hy);
            save_laz(file_u.substr(0,file_u.find(".laz"))+to_string(bore_roll)+"_"+to_string(bore_pitch)+"_"+to_string(bore_yaw)+"_hatp.laz", point_cloud_data, em);
            save_laz(file_u.substr(0,file_u.find(".laz"))+to_string(bore_roll)+"_"+to_string(bore_pitch)+"_"+to_string(bore_yaw)+"_barp.laz", point_cloud_model, em);
            save_laz(file_u.substr(0,file_u.find(".laz"))+to_string(bore_roll)+"_"+to_string(bore_pitch)+"_"+to_string(bore_yaw)+".laz", lidar_point_cloud, em);
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
        
        apply_transform_new_order(bore_roll*pi/180, bore_pitch*pi/180, bore_yaw*pi/180, lidar_point_cloud, uav_cloud_u, roll_pitch_yaw, scanner_x, scanner_y, scanner_z, hr, hp, hy);
        if(format_laz){
            save_laz(file_u.substr(0,file_u.find(".laz"))+to_string(bore_roll)+"_"+to_string(bore_pitch)+"_"+to_string(bore_yaw)+".laz", lidar_point_cloud, em);
        }
        else{
            save_laz(file_u.substr(0,file_u.find(".laz"))+to_string(bore_roll)+"_"+to_string(bore_pitch)+"_"+to_string(bore_yaw)+".las", lidar_point_cloud, em);
        }
    }
#ifdef USE_MATPLOT
    plot(point_cloud_model, point_cloud_data);
#endif
    
}

