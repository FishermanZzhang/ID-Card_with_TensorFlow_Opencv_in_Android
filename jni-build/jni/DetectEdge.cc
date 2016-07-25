#include "DetectEdge.h"

DetectEdge::DetectEdge(){}
DetectEdge::~DetectEdge(){}

void DetectEdge::MergeLines(vector<cv::Vec4i>& lines,cv::Vec4i& edge, int type){
  if (type == 1)  {
    MergeVerticalLines(lines, edge); // 垂直线
  }
  else{
    MergeHorizontalLines(lines, edge); //水平线
  }
}
void DetectEdge::MergeHorizontalLines(vector<cv::Vec4i>& lines, cv::Vec4i& edge){
  //水平直线合并策略
  //并不合并，返回最长的直线
  auto distancepowerfromVec4i = [](cv::Vec4i& line1){
    return (pow(abs(line1[0] - line1[2]), 2) + pow(abs(line1[1] - line1[3]), 2));
  };
  double dis = 0.0;
  for (size_t i = 0; i < lines.size(); ++i){
    double tdis = 0.0;
    if ((tdis = distancepowerfromVec4i(lines[i])) > dis){
      dis = tdis;
      edge = lines[i];
    }
  }
}
void DetectEdge::MergeVerticalLines(vector<cv::Vec4i>& lines, cv::Vec4i& edge){
  //垂直直线转成水平直线，调用水平直线合并策略
  vector<cv::Vec4i> tmplines(lines.size());
  for (size_t i = 0; i < lines.size(); ++i){
    cv::Vec4i line = lines[i];
    std::swap(line[0], line[1]);
    std::swap(line[2], line[3]);
    tmplines.emplace_back(line);
  }
  MergeHorizontalLines(tmplines, edge);
  std::swap(edge[0], edge[1]);
  std::swap(edge[2], edge[3]);
}

cv::Point DetectEdge::CrossPoint(const cv::Vec4i& line1, const cv::Vec4i& line2){
  cv::Point p;
  float denominator = (line1[1] - line1[3])*(line2[2] - line2[0]) - (line1[2] - line1[0])*(line2[1] - line2[3]);
  p.x = (line1[1] * line1[2] - line1[0] * line1[3])*(line2[2] - line2[0]) - (line1[2] - line1[0])*(line2[1] * line2[2] - line2[0] * line2[3]);
  p.x = p.x / denominator;
  p.y = (line1[1] - line1[3])*(line2[1] * line2[2] - line2[0] * line2[3]) - (line2[1] - line2[3])*(line1[1] * line1[2] - line1[0] * line1[3]);
  p.y = p.y / denominator;
  return p;
}
