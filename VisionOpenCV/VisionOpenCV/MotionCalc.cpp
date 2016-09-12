#include "MotionCalc.h"
#include <iostream>

using namespace std;
#define M_PI       3.14159265358979323846
#define MOVE_MAX_SPEED 0.05	//3度
#define MOVE_NORMAL_SPEED 0.02	//1度
#define MOVE_EACH_TIME	10		//每次移动时间（脉冲数）
#define	SPEED_THRESHOLD_ANGLE 0.7	//40度
#define VISION_SECTION_COUNT	5	//视角 分区域 总数
#define ONE_SIDE_SECTION_BOUND	0.1	//边界缓冲区域 占比

//当前，目标角度（实时）
double MotionCalc::currentAngle, MotionCalc::targetAngle;
//分区域相关 角度
double MotionCalc::sectionAngle, MotionCalc::sectionBoundAngle, MotionCalc::moveSpeed;

bool MotionCalc::moveFlag;

MotionCalc::MotionCalc(int imageWidth):MAX_VISION_ANGLE(60)
{
	videoImageWidth = imageWidth;
	verticalDistance = (imageWidth / 2.0) / tan(MAX_VISION_ANGLE / 2 * M_PI / 180);
	sectionAngle = MAX_VISION_ANGLE / VISION_SECTION_COUNT * M_PI / 180;
	sectionBoundAngle = sectionAngle * ONE_SIDE_SECTION_BOUND;
	moveFlag = false;
}


MotionCalc::~MotionCalc(void)
{
}


double MotionCalc::CalcAngleByLocation(int xValue)
{
	//计算相对中线距离
	double horiLen = xValue - videoImageWidth / 2.0;
	//通过反正切计算弧度
	double angle = atan2(fabs(horiLen),verticalDistance);
	//角度的方向：摄像头对称成像，所以正负反过来了
	angle *= horiLen>0?1:-1;
	//转换为角度
	return angle;
}

double  MotionCalc::CalcAngleNextStep()
{//直接移向目标位置
	double tmpDiff = fabs(targetAngle - currentAngle);
	int tmpFlag = targetAngle > currentAngle ? 1 : -1;

	if(tmpDiff > MOVE_NORMAL_SPEED)
	{
		if(tmpDiff < SPEED_THRESHOLD_ANGLE)
			currentAngle += MOVE_NORMAL_SPEED * tmpFlag;
		else
			currentAngle += MOVE_MAX_SPEED * tmpFlag;
	}
	return currentAngle;
}

double  MotionCalc::CalcAngleNextStepBySection()
{
	if(!moveFlag)
	{//移动中
		int currentSection = currentAngle / sectionAngle, targetSection = targetAngle / sectionAngle;
		int sectionDiff = targetSection - currentSection;
		int sectionCount = abs(sectionDiff);
		double overLen = currentAngle - sectionAngle * currentSection;
		double tmpValue = targetAngle - max(currentSection, targetSection) * sectionAngle;
		moveFlag = true;
		if(sectionCount > 1 || sectionCount == 1 && abs(tmpValue) > sectionBoundAngle)
			moveSpeed = sectionCount * sectionAngle / MOVE_EACH_TIME * (sectionDiff>0?1:-1);
		else if(currentSection == 0 && targetSection ==0 && currentAngle * targetAngle < 0 && abs(targetAngle) > sectionBoundAngle)
			moveSpeed = 1 * sectionAngle / MOVE_EACH_TIME * (targetAngle>0?1:-1);
		else{
			moveSpeed = 0;
			moveFlag = false;
		}
	}
	currentAngle += moveSpeed;

	if(((currentAngle - targetAngle) * moveSpeed) >= 0)
	{//到达目的地
		moveFlag = false;
	}
	return currentAngle;
}

void MotionCalc::MoveOrigin()
{
	currentAngle = 0;
}

void MotionCalc::setAngleTarget(double degree)
{
	if(!moveFlag)
		targetAngle = degree;	
}

	