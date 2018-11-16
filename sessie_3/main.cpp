#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, const char** argv)
{
    /// Adding a little help option and command line parser input
    CommandLineParser parser(argc, argv,
        "{ help h usage ? || show this message }"
        "{ image i1|| (required) path to image }"
        "{ template t1|| (required) path to image }"
    );

    if (parser.has("help")){
        parser.printMessage();
        return 0;
    }

    /// Collect data from arguments
    string image_image_loc(parser.get<string>("image"));
    string image_templ_loc(parser.get<string>("template"));
    if (image_image_loc.empty() || image_templ_loc.empty()){
        cerr << "error while reading your images, check if you put the correct path.";
        parser.printMessage();
        return -1;
    }

    /// Loading the image and showing it on screen
    Mat image1= imread(image_image_loc);
    if(image1.empty()) {
        cerr << "error while loading your images, check if you put the correct path.";
        return -1;
    }
    //imshow("Reference", image1);
    //waitKey(0);

    /// Loading the template image
    Mat templ= imread(image_templ_loc);
    if(templ.empty()) {
        cerr << "error while loading your images, check if you put the correct path.";
        return -1;
    }

    /// Template matching
    Mat greyImg;
    Mat greyTempl;

    cvtColor(image1, greyImg, CV_BGR2GRAY);
    cvtColor(templ, greyTempl, CV_BGR2GRAY);

    Mat result;
    matchTemplate( greyImg, greyTempl, result, TM_SQDIFF );
    normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );

    double minVal; double maxVal; double threshold = 0.9; Point minLoc; Point maxLoc; Point matchLoc;

    for(int k=1;k<=100;k++)
    {
        minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
        result.at<float>(minLoc.x,minLoc.y)=1.0;
        result.at<float>(maxLoc.x,maxLoc.y)=0.0;

        matchLoc = maxLoc;
        if (maxVal >= threshold) {
            rectangle( greyImg, matchLoc, Point( matchLoc.x + greyTempl.cols , matchLoc.y + greyTempl.rows ), Scalar(0,0,0), 2, 8, 0 );
            rectangle( result, matchLoc, Point( matchLoc.x + greyTempl.cols , matchLoc.y + greyTempl.rows ), Scalar(0,0,0), 2, 8, 0 );
        }
        else
            break;
    }

    imshow("original", greyImg);
    waitKey(0);
    imshow("result", result);
    waitKey(0);

    Mat mask, temp2;
    threshold(result, mask, 0.9 ,1, THRESH_BINARY);
    mask.convertTo(mask, CV_8UC1);
    mask = mask * 255;
    imshow("threshold img", mask);
    waitKey(0);

    for(int row = 0; row < mask.rows; row++) {
        for(int column = 0; column < mask.cols; column++) {
            if(mask.at<uchar>(row,column) == 255) {
                rectangle(temp2, Point(column, row), Point(column + templ.cols, row + templ.rows), Scalar(0,0,0), 2, 8, 0 );
            }
        }
    }

    vector<vector<Point>> contours;
    findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    for(size_t i=0; i < contours.size(); i++) {
        Rect region = boundingRect(contours[i]);
        Mat temp = result(region);
        Point maxLoc;
        minMaxLoc( temp, NULL, NULL, NULL, &maxLoc);
        rectangle(greyImg, Point(region.x + maxLoc.x, region.y + maxLoc.y), Point(region.x + maxLoc.x + templ.cols , region.y + maxLoc.y + temp.rows), Scalar(0,0,0), 2, 8, 0);
    }
    imshow("find cont", greyImg);
    waitKey(0);

    return 0;
}
