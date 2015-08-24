#include "tracker.hpp"

cv::Ptr<Tracker> createTrackerDummy();
cv::Ptr<Tracker> createTrackerKustikova();

cv::Ptr<Tracker> createTracker(const std::string &impl_name)
{
    if (impl_name == "dummy")
        return createTrackerDummy();
    if (impl_name == "kustikova")
         return createTrackerKustikova();

    return 0;
}
