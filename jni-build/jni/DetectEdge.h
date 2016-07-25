#ifndef _DETECTEDGE_H_
#define _DETECTEDGE_H_
#include<opencv2/opencv.hpp>
#include<vector>
using namespace std;
class DetectEdge
{
  public:
    DetectEdge();
    virtual ~DetectEdge();
    virtual void detect(cv::Mat& image, vector<cv::Vec4i>& edges) = 0;
    virtual bool detect(cv::Mat& image, cv::Mat& dst) = 0;
    void MergeLines(vector<cv::Vec4i>& lines, cv::Vec4i& edge, int type);
    cv::Point CrossPoint(const cv::Vec4i& line1, const cv::Vec4i& line2);
  private:
    void MergeHorizontalLines(vector<cv::Vec4i>& lines, cv::Vec4i& edge);
    void MergeVerticalLines(vector<cv::Vec4i>& lines, cv::Vec4i& edge);
};
#endif
