#include <tracker.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>

using namespace cv;
using namespace std;

class TrackerMoshkina : public Tracker
{
 public:
    virtual ~TrackerMoshkina() {}

    virtual bool init( const cv::Mat& frame, const cv::Rect& initial_position );
    virtual bool track( const cv::Mat& frame, cv::Rect& new_position );

	cv::Mat prevFrame;

 private:
    cv::Rect position_;
	
};

bool TrackerMoshkina::init( const cv::Mat& frame, const cv::Rect& initial_position )
{
    position_ = initial_position;
	return true;
}

bool TrackerMoshkina::track( const cv::Mat& frame, cv::Rect& new_position )
{
	if (prevFrame.empty()) {
		frame.copyTo(prevFrame); return 1;
	}

	if (frame.empty()) {
		return 1;
	}

	cv::Mat gray, prevGray;
	cvtColor(frame, gray, COLOR_BGR2GRAY);
	cvtColor(prevFrame, prevGray, COLOR_BGR2GRAY);
    new_position = position_;

	std::vector<cv::Point2f> points, new_points;
	vector<uchar> status;
    vector<float> err, shifts_x, shifts_y, shifts_relations;

	

	//choose points inside the rect
	for (int i = 0; i<position_.width; i+=10)
		for (int j = 0; j<position_.height; j+=10)
		{ 
			Point2f new_p = Point(position_.x+i,position_.y+j);
			points.push_back(new_p);
			cout << position_.x+i << " " << position_.y+j << "\n";
		}
		
	calcOpticalFlowPyrLK(prevGray, gray, points, new_points, status, err);

	cout << points.size() << " " << new_points.size() << " " << status.size() << " " << err.size() << "\n";

	//delete lost points
	for (int i=status.size()-1; i>0; i--)
	{
		if (status[i]==0) 
		{
			points.erase(points.begin()+i);
			new_points.erase(new_points.begin()+i);
			err.erase(err.begin()+i);
			
		}
	}

	 

	//sorting errors and points according to them
	for (int j=0; j<err.size()-1; j++) {
		for (int i=0; i<err.size()-j-1; i++) {
			if (err[i]>err[i+1]) {
				float tmp_err;
				Point2f tmp_new_point, tmp_point;
				tmp_err = err[i];
				err[i]=err[i+1];
				err[i+1]=tmp_err;

				tmp_new_point = new_points[i];
				new_points[i]=new_points[i+1];
				new_points[i+1]=tmp_new_point;

				tmp_point = points[i];
				points[i]=points[i+1];
				points[i+1]=tmp_point;
			}
	}
	}

	//deleting halfs of sorted vectors
	for (int i = err.size()-1; i> points.size()/2; i--) {
		points.erase(points.begin()+i);
		new_points.erase(new_points.begin()+i);
		err.erase(err.begin()+i);
	}

	//calculating x and y shifts
	for (int i=0; i<points.size(); i++) {
		float shift_x = new_points[i].x-points[i].x;
		float shift_y = new_points[i].y-points[i].y;
		shifts_x.push_back(shift_x);
		shifts_y.push_back(shift_y);
	}

	//calculating scale
	/*for (int i=0; i<points.size(); i++) {
		for (int j=i+1; j<points.size(); j++) {
		float new_shift = sqrt(pow((new_points[j].x-new_points[i].x), 2) + pow((new_points[j].y-new_points[i].y), 2));
		float shift = sqrt(pow((points[j].x-points[i].x), 2) + pow((points[j].y-points[i].y), 2));
		shifts_relations.push_back(new_shift/shift);
		}
	}*/

	//sort(shifts_relations.begin(), shifts_relations.end());
	//sort(shifts_x.begin(), shifts_x.end());
	//sort(shifts_y.begin(), shifts_y.end());
	
	//float newx, newy, newwidth, newheight; 
	//newwidth = position_.width*shifts_relations[shifts_relations.size()/2];
	//newheight = position_.height*shifts_relations[shifts_relations.size()/2];
	//if (shifts_x[shifts_x.size()/2] < 0)
	//{
	//	newx = (position_.x+shifts_x[shifts_x.size()/2] < 0) ? 0 : position_.x+shifts_x[shifts_x.size()/2];
	//}
	//else
	//{
	//	newx = (position_.x+shifts_x[shifts_x.size()/2] >= frame.cols) ? -1 : position_.x+shifts_x[shifts_x.size()/2];
	//}
	//newy = position_.y+shifts_y[shifts_y.size()/2];
	
	Point2f p1 = points[0];
	Point2f p2 = new_points[0];
	float dy = p1.y - position_.y;
	float dx = p1.x - position_.x;
	float newx = p2.x - dx;
	float newy = p2.y - dy;
	float newwidth = position_.width;
	float newheight = position_.height;

	new_position = Rect(newx, newy, newwidth, newheight);

	position_=new_position;
	frame.copyTo(prevFrame);

	return true;
}

cv::Ptr<Tracker> createTrackerMoshkina()
{
    return cv::Ptr<Tracker>(new TrackerMoshkina());
}
