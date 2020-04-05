
#include <ros/ros.h>
#include <mavros_msgs/CommandBool.h>
#include <mavros_msgs/SetMode.h>
#include <mavros_msgs/State.h>
#include <mavros_msgs/AttitudeTarget.h>
#include <mavros_msgs/PositionTarget.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/TwistStamped.h>
#include <mavros_msgs/ActuatorControl.h>
#include <sensor_msgs/Imu.h>
#include "formation_simulation/communication.h"
#include <bitset>
#include <Eigen/Eigen>
using namespace std;

mavros_msgs::State current_state_uav3;
Eigen::Vector3d pos_drone_uav3;
Eigen::Vector3d vel_drone_uav3;
Eigen::Vector3d takeoff_position_uav3 = Eigen::Vector3d(0.0,0.0,0.0);
mavros_msgs::PositionTarget pos_setpoint3;
double yaw_sp = 0;

//其他无人机信息
Eigen::Vector3d pos_drone_uav0;
Eigen::Vector3d vel_drone_uav0;

Eigen::Vector3d pos_drone_uav1;
Eigen::Vector3d vel_drone_uav1;

Eigen::Vector3d pos_drone_uav2;
Eigen::Vector3d vel_drone_uav2;

Eigen::Vector3d pos_drone_uav4;
Eigen::Vector3d vel_drone_uav4;

void Uav3PositionSubscriberCallback(const geometry_msgs::PoseStamped::ConstPtr &msg)
{
    pos_drone_uav3  = Eigen::Vector3d(msg->pose.position.x,msg->pose.position.y,msg->pose.position.z);
}

void Uav3VelocitySubscriberCallback(const geometry_msgs::TwistStamped::ConstPtr &msg)
{
    vel_drone_uav3 = Eigen::Vector3d(msg->twist.linear.x,msg->twist.linear.y,msg->twist.linear.z);
}

void OtherUavSubscriberCallback(const formation_simulation::communication::ConstPtr &msg)
{
    pos_drone_uav0 = Eigen::Vector3d(msg->position[0].x,msg->position[0].y,msg->position[0].z);
    pos_drone_uav1 = Eigen::Vector3d(msg->position[1].x,msg->position[1].y,msg->position[1].z);
    pos_drone_uav2 = Eigen::Vector3d(msg->position[2].x,msg->position[2].y,msg->position[2].z);
    pos_drone_uav4 = Eigen::Vector3d(msg->position[3].x,msg->position[3].y,msg->position[3].z);

    vel_drone_uav0 = Eigen::Vector3d(msg->velocity[0].x,msg->velocity[0].y,msg->velocity[0].z);
    vel_drone_uav1 = Eigen::Vector3d(msg->velocity[1].x,msg->velocity[1].y,msg->velocity[1].z);
    vel_drone_uav2 = Eigen::Vector3d(msg->velocity[2].x,msg->velocity[2].y,msg->velocity[2].z);
    vel_drone_uav4 = Eigen::Vector3d(msg->velocity[3].x,msg->velocity[3].y,msg->velocity[3].z);
}


int main(int argc, char **argv)
{
    ros::init(argc, argv, "formation_uav3_control");
    ros::NodeHandle nh("~");

    ros::Subscriber uav3_position_sub = nh.subscribe<geometry_msgs::PoseStamped>("/uav3/mavros/local_position/pose", 100, Uav3PositionSubscriberCallback);
    ros::Subscriber uav3_velocity_sub = nh.subscribe<geometry_msgs::TwistStamped>("/uav3/mavros/local_position/velocity_local", 100, Uav3VelocitySubscriberCallback);
    ros::Subscriber oher_uav_sub = nh.subscribe<formation_simulation::communication>("/uav3_sim/uav3_communication/pos_vel/local", 100, OtherUavSubscriberCallback);
    ros::Publisher uav3_setpoint_raw_local_pub = nh.advertise<mavros_msgs::PositionTarget>("/uav3/mavros/setpoint_raw/local", 10);

    ros::Rate rate(10.0);

    // 先读取一些飞控的数据
    int i = 0;
    for(i = 0; i < 20; i++)
    {
        ros::spinOnce();
        rate.sleep();
    }

    takeoff_position_uav3 = pos_drone_uav3;
    ros::Time begin_time = ros::Time::now();
    cout<<"uav3 command node started!!!"<<endl;
    while(ros::ok())
    {
        //执行回调函数
        ros::spinOnce();
        pos_setpoint3.header.stamp = ros::Time::now();

        pos_setpoint3.type_mask = 0b100111111000;  // 100 111 111 000  xyz + yaw
        
        pos_setpoint3.coordinate_frame = 1;

        pos_setpoint3.position.x = takeoff_position_uav3[0];
        pos_setpoint3.position.y = takeoff_position_uav3[1];
        pos_setpoint3.position.z = takeoff_position_uav3[2]+5;

        pos_setpoint3.yaw = yaw_sp;

        uav3_setpoint_raw_local_pub.publish(pos_setpoint3);

        rate.sleep();
    }
    return 0;
}

