#include <tracker.hpp>
#include <opencv2/opencv.hpp>

static cv::Mat previous_frame;

class TrackerDummy : public Tracker
{
 public:
    virtual ~TrackerDummy() {}

    virtual bool init( const cv::Mat& frame, const cv::Rect& initial_position );
    virtual bool track( const cv::Mat& frame, cv::Rect& new_position );

 private:
    cv::Rect position_;
};

bool TrackerDummy::init( const cv::Mat& frame, const cv::Rect& initial_position )
{
    position_ = initial_position;
	return true;
}

bool TrackerDummy::track( const cv::Mat& frame, cv::Rect& new_position )
{
    new_position = position_;
	return true;
}

cv::Ptr<Tracker> createTrackerDummy()
{
    return cv::Ptr<Tracker>(new TrackerDummy());
}

class TrackerIlya : public Tracker
{
 public:
    virtual ~TrackerIlya() {}

    virtual bool init( const cv::Mat& frame, const cv::Rect& initial_position );
    virtual bool track( const cv::Mat& frame, cv::Rect& new_position );

 private:
    cv::Rect position_;
};

bool TrackerIlya::init( const cv::Mat& frame, const cv::Rect& initial_position )
{
	previous_frame=frame;
    position_ = initial_position;
	return true;
}

bool TrackerIlya::track( const cv::Mat& frame, cv::Rect& new_position )
{
    float x=position_.x;
    float y=position_.y;
    long width=position_.width;
    long height=position_.height;
	cv::Point2f p;
	std::vector<cv::Point2f> prev, next;
	for(long i=0;i<height;i++)
	{
		for(long j=0;j<width;j++)
		{
			p.y=(y+i);
			p.x=(x+j);
			prev.push_back(p);
		}
	}
	/*////
	///////*
	////cv::Mat copySrc;
	////frame.copyTo(copySrc);

	////cv::namedWindow("test points");
	////for (int i = 0; i < prev.size(); i+=5)
	////{
	////	cv::circle(copySrc, prev[i], 2, cv::Scalar(255, 0, 0));
	////}
	////imshow("test points", copySrc);
	//////*/

	std::vector<unsigned char> status;
	std::vector<float> err;
	cv::calcOpticalFlowPyrLK(previous_frame,frame,prev,next,status,err );

	/*/*cv::Mat copyFrame;
	frame.copyTo(copyFrame);*/

	/*////cv::namedWindow("test points next");
	//for (int i = 0; i < next.size(); i+=5)
	//{
	//	cv::circle(copyFrame, next[i], 2, cv::Scalar(255, 0, 0));
	//}
	////imshow("test points next", copyFrame);
	////cv::waitKey();

	//for(long i=height/2;i<height;i++)
	//{
	//	for(long j=0;j<width;j++) 
	//	{
	//		//std::cout << prev[i*width+j].x << std::endl;
	//		//std::cout << next[i*width+j].x << std::endl;
	//		std::cout<<(next[i*width+j].x-prev[i*width+j].x)<<"\n";
	//	}
	//}
			/*for(long i=0;i<height;i++){
								for(long j=0;j<width;j++) {next[i*width+j].x-=prev[i*width+j].x;
								next[i*width+j].y-=prev[i*width+j].y;								  }}
			for(long i=0;i<next.size();i++){std::cout<<next[i].x<<"\n";}*/

	/*/*std::vector<bool> delet;for(long i=0;i<height;i++)for(long j=0;j<width;j++) delet.push_back(false);
	for(long i=0;i<(width*height);i++){if(h[i]==0) delet[i]=true;}
	{long tek=0;for(long  i=0;i<(width*height);i++) if(delet[i]){next.erase(next.begin()+tek);}else{tek++;}}

	float dx;
	std::vector<float> ddx;for(long i=0;i<next.size();i++) ddx.push_back(next[i].x);
	std::sort(ddx.begin(),ddx.end());
	dx=ddx[ddx.size()/2];*/
/*	//std::cout<<dx<<"\n";
	//for(;;){}*/
    for(long i=0;i<prev.size();i++){if(!status[i]) prev.erase(prev.begin()+i);}
    for(long i=0;i<10000;i++)for(long j=0;j<(prev.size()-1);j++){
                                                                if(err[j]>err[j+1]){
                                                                    cv::Point2f t=next[j];next[j]=next[j+1];next[j+1]=t;
                                                                    t=prev[j];prev[j]=prev[j+1];prev[j+1]=t;
                                                                    float tt;tt=err[j];err[j]=err[j+1];err[j+1]=tt;}
                                                                }
    std::vector<float> dx;
    for(long i=0;i<prev.size()/2;i++) dx.push_back(next[i].x-prev[i].x);

    std::vector<float> dy;
    for(long i=0;i<prev.size()/2;i++) dy.push_back(next[i].y-prev[i].y);
    std::sort(dx.begin(),dx.end());
    std::sort(dy.begin(),dy.end());
    x+=dx[dx.size()/2];
    y+=dy[dy.size()/2];
    {
    cv::Rect t(x,y,width,height);
    position_=t;
    }
    new_position = position_;
    previous_frame=frame;
    return true;
}

cv::Ptr<Tracker> createTrackerIlya()
{
    return cv::Ptr<Tracker>(new TrackerIlya());
}
