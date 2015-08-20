#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <tracker.hpp>
#include <math.h>

class TrackerMedianFlow : public Tracker
{
 public:
    virtual ~TrackerMedianFlow() {}

    virtual bool init( const cv::Mat& frame, const cv::Rect& initial_position );
    virtual bool track( const cv::Mat& frame, cv::Rect& new_position );

 private:
    cv::Mat curFrame;
    cv::Rect position_;
};

void sort(std::vector<float>& A)
{
    for (int j = 0; j < A.size() - 1; j++) 
    {
         for (int i = 0; i < A.size() - j - 1; i++) 
         {
             if (A[i] > A[i+1]) 
             {
                 std::swap(A[i], A[i+1]);
             }
         }
    }
}

float distance(cv::Point2f pt1, cv::Point2f pt2)
{
    return sqrt( (pt2.x - pt1.x) * (pt2.x - pt1.x) + (pt2.y - pt1.y) * (pt2.y - pt1.y) );
}

bool TrackerMedianFlow::init( const cv::Mat& frame, const cv::Rect& initial_position )
{
    position_ = initial_position;
	return true;
}

bool TrackerMedianFlow::track( const cv::Mat& frame, cv::Rect& new_position )
{
    if (frame.empty())
    {
        std::cout << "Image error" << std::endl;
        return 0;
    }

    if (curFrame.empty())
    {
        frame.copyTo(curFrame);
        return 0;
    } 

    //Choose points
    std::vector<cv::Point2f> nextPts;
    std::vector<cv::Point2f> prevPts;
    cv::Mat oldGray, newGray;
    /*for (int i = 0; i < position_.width; i += 10)
        for (int j = 0; j < position_.height; j += 10)
        {
            cv::Point2f pt(i + position_.x, j + position_.y);
            prevPts.push_back(pt);
            nextPts.push_back(pt);
        }*/

    //Optical Flow
    cv::cvtColor(curFrame, oldGray, cv::COLOR_BGR2GRAY);
    cv::cvtColor(frame, newGray, cv::COLOR_BGR2GRAY);

    cv::Mat roi(oldGray, position_);
    cv::goodFeaturesToTrack(roi, prevPts, 50, 0.01, 10);
    std::vector<uchar> status;
    std::vector<float> err;
    cv::calcOpticalFlowPyrLK(oldGray, newGray, prevPts, nextPts, status, err);

     if (prevPts.size() == 0)
    {
        std::cout << "Object lost" << std::endl;
        return 0;
    }

    //Delete lost points
    for (int i = status.size() - 1; i >= 0; i--)
    {
        if (!status[i])
        {
            prevPts.erase(prevPts.begin() + i);
            nextPts.erase(nextPts.begin() + i);
            err.erase(err.begin() + i);
        }
    }

    //Calculate median error
    for (int j = 0; j < err.size() - 1; j++) 
    {
         for (int i = 0; i < err.size() - j - 1; i++) 
         {
             if (err[i] > err[i+1]) 
             {
                 std::swap(err[i], err[i+1]);
                 std::swap(nextPts[i], nextPts[i+1]);
                 std::swap(prevPts[i], prevPts[i+1]);
             }
         }
     }
    float median_err = err[err.size()/2];

    //Delete "bad" points. Calculate median shifts
    std::vector<float> XShift, YShift;
    for (int i = err.size() - 1; i >= 0; i--)
    {
        if (err[i] > median_err)
        {
            err.erase(err.begin() + i);
            nextPts.erase(nextPts.begin() + i);
            prevPts.erase(prevPts.begin() + i);
        }
        else
        {
            XShift.push_back(nextPts[i].x - prevPts[i].x);
            YShift.push_back(nextPts[i].y - prevPts[i].y);
        }
    }
    sort(XShift);
    sort(YShift);
    float XMedianShift = XShift[XShift.size() / 2];
    float YMedianShift = YShift[YShift.size() / 2];

    cv::Mat im;
    frame.copyTo(im);
    for (int i = 0; i < nextPts.size(); i++)
    {
        cv::circle(im, nextPts[i], 3, cv::Scalar(255, 255, 0));
    }
    cv::imshow("debug", im);

    //Calculate median distance ratio
    std::vector<float> distanceRatios;
    for (int i = 0; i < prevPts.size(); i++)
        for (int j = i; j < prevPts.size(); j++)
        {
            float distanceRatio = distance(nextPts[i], nextPts[j]) / distance(prevPts[i], prevPts[j]);
            distanceRatios.push_back(distanceRatio);
        }
    sort(distanceRatios);
    float medianDistanceRatio = distanceRatios[distanceRatios.size() / 2];

    new_position = cv::Rect(position_.x + XMedianShift, position_.y + YMedianShift, position_.width * medianDistanceRatio, position_.height * medianDistanceRatio);
    position_ = new_position;
    frame.copyTo(curFrame);

	return true;
}

cv::Ptr<Tracker> createTrackerMedianFlow()
{
    return cv::Ptr<Tracker>(new TrackerMedianFlow());
}
