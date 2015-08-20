#pragma once

#include <sstream>
#include <opencv2/highgui/highgui.hpp>

class GUI
{
private:
    std::string window_name_;

    cv::Rect bounding_box_;
    cv::Point p1_, p2_;
    cv::Mat  init_image_;
    cv::Mat  display_image_;
    bool start_selection_;
    bool object_selected_;

public:
    GUI(const std::string &window_name = "Tracking Sample")
        : window_name_(window_name)
    {
        cv::namedWindow(window_name_);
        cv::setMouseCallback(window_name_, onMouse, this);
    }

    ~GUI()
    {
        cv::destroyWindow(window_name_);
    }

    cv::Rect initBoundingBox(const cv::Rect &bounding_box,
                             const cv::Mat &init_image)
    {
        if (bounding_box != cv::Rect() &&
            (bounding_box & cv::Rect(0, 0, init_image.cols, init_image.rows)) == bounding_box)
        {
            // if provided rect is valid, save it

            bounding_box_ = bounding_box;
            object_selected_ = true;
        }
        else
        {
            // otherwise, ask user to draw the bounding box

            bounding_box_ = cv::Rect();

            init_image.copyTo(init_image_);
            init_image.copyTo(display_image_);

            start_selection_ = false;
            object_selected_ = false;

            while(!object_selected_ &&
                  displayImage(init_image_, bounding_box_))
                continue;
        }

        return bounding_box_;
    }

    bool displayImage(const cv::Mat &image,
                      const cv::Rect &rect,
                      const cv::Scalar &rect_color = cv::Scalar( 0, 255, 0 ),
                      const cv::Rect &gt = cv::Rect()
        )
    {
        image.copyTo(display_image_);
        if (gt != cv::Rect())
            cv::rectangle(display_image_, gt, cv::Scalar( 255, 0, 0 ), 2, 1);
        cv::rectangle(display_image_, rect, rect_color, 2, 1);
        cv::imshow(window_name_, display_image_);
        char c = cv::waitKey(30) & 0xFF;
        if (c == 27) // ESC
            return false;

        return true;
    }

    static void onMouse( int event, int x, int y, int, void *s)
    {
        GUI *self = reinterpret_cast<GUI *>(s);

        if( !self->object_selected_ )
        {
            switch ( event )
            {
            case cv::EVENT_LBUTTONDOWN:
                self->start_selection_ = true;
                self->p1_ = cv::Point(x, y);
                break;
            case cv::EVENT_LBUTTONUP:
                self->object_selected_ = true;
                self->start_selection_ = false;
                break;
            case cv::EVENT_MOUSEMOVE:
                if( self->start_selection_ && !self->object_selected_ )
                {
                    self->p2_ = cv::Point(x, y);
                    self->bounding_box_ = cv::Rect(self->p1_, self->p2_);
                }
                break;
            }
        }
    }
};
