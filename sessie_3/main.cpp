#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

void singleResult(Mat image1, Mat templ);
void multipleResults(int, void*);
void rotationInvariant();

Mat image1; Mat templ;
int match_method = 3;
int max_angle = 360;
int step_angle = 30;

// Return the rotation matrices for each rotation
// The angle parameter is expressed in degrees!
void rotate(Mat& src, double angle, Mat& dst)
{
    Point2f pt(src.cols/2., src.rows/2.);
    Mat r = getRotationMatrix2D(pt, angle, 1.0);
    warpAffine(src, dst, r, Size(src.cols, src.rows));
}

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
    image1= imread(image_image_loc);
    if(image1.empty()) {
        cerr << "error while loading your images, check if you put the correct path.";
        return -1;
    }
    imshow("Loaded image", image1);
    waitKey(0);

    /// Loading the template image
    templ= imread(image_templ_loc);
    if(templ.empty()) {
        cerr << "error while loading your images, check if you put the correct path.";
        return -1;
    }

    imshow("Loaded template", templ);
    waitKey(0);

    singleResult(image1, templ);

    /// Create windows
    namedWindow( "Result", CV_WINDOW_AUTOSIZE );

    /// Create Trackbar
    createTrackbar( "Method: \n 0: SQDIFF \n 1: SQDIFF NORMED \n 2: TM CCORR \n 3: TM CCORR NORMED \n 4: TM COEFF \n 5: TM COEFF NORMED", "Result", &match_method, 5, multipleResults );

    multipleResults(0, NULL);
    //rotationInvariant();

    waitKey(0);

    return 0;
}

/// Template matching (single result)
void singleResult(Mat image1, Mat templ)
{
    Mat greyImg;
    Mat greyTempl;

    /// Converting to grey image
    cvtColor(image1, greyImg, CV_BGR2GRAY);
    cvtColor(templ, greyTempl, CV_BGR2GRAY);

    /// Templatematching and normalizing
    Mat result;
    matchTemplate( greyImg, greyTempl, result, TM_SQDIFF );
    normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );

    double minVal; double maxVal; Point minLoc; Point maxLoc; Point matchLoc;

    minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

    matchLoc = minLoc;

    /// Drawing a rectangle around the found match
    rectangle( greyImg, matchLoc, Point( matchLoc.x + greyTempl.cols , matchLoc.y + greyTempl.rows ), Scalar(0,0,0), 2, 8, 0 );
    //rectangle( result, matchLoc, Point( matchLoc.x + greyTempl.cols , matchLoc.y + greyTempl.rows ), Scalar(0,0,0), 2, 8, 0 );

    imshow("Original to Grey", greyImg);
    waitKey(0);

    imshow("Result (single)", result);
    waitKey(0);
}

/// Template matching (multiple results)
void multipleResults(int, void*)
{
    Mat result;
    Mat img_display;

    image1.copyTo(img_display);

    /// Templatematching and normalizing
    matchTemplate( img_display, templ, result, match_method );
    normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );

    /// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
    if( match_method  == TM_SQDIFF || match_method == TM_SQDIFF_NORMED )
        { result = 1 - result; }

    imshow("Result", result);

    double th = 0.8;

    /// Thresholding
    Mat mask;
    threshold(result, mask, th ,1, THRESH_BINARY);
    mask.convertTo(mask, CV_8UC1);
    mask = mask*255;

    imshow("Threshold image", mask);

    /// Finding local matches so all matches are found instead of just one
    vector<vector<Point>> contours;
    findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    for(size_t i=0; i < contours.size(); i++) {
        Rect region = boundingRect(contours[i]);
        Mat temp = result(region);
        Point matchLoc;
        minMaxLoc( temp, NULL, NULL, NULL, &matchLoc, Mat());
        rectangle(img_display, Point(region.x + matchLoc.x, region.y + matchLoc.y), Point(region.x + matchLoc.x + templ.cols , region.y + matchLoc.y + templ.rows), Scalar(0,0,0), 2, 8, 0);
    }

    imshow("Bounding rectangles", img_display);
}
/* Werkt niet
void rotationInvariant()
{
    Mat result;
    Mat img_display;

    image1.copyTo(img_display);

    /// Create the 3D model matrix of the input image
    int steps = max_angle / step_angle;
    vector<Mat> rotated_images;

    for (int i = 0; i < steps; i ++){
        /// Rotate the image
        Mat rotated = img_display.clone();
        /// Preprocess the images already, since we will need it for detection purposes
        rotate(img_display, (i+1)*step_angle, rotated);
        // Add to the collection of rotated and processed images
        rotated_images.push_back(rotated);
    }

    // Perform detection on each image seperately
    // Store all detection in a vector of vector detections
    RNG rng(12345);
    for (int j = 0; j < (int)rotated_images.size(); j++){
        // You can reduce the scale range, downscale step or score threshold here if prefered
        /// Templatematching and normalizing
        matchTemplate( rotated_images[j], templ, result, TM_CCOEFF_NORMED );
        normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );
        Mat mask;
        threshold(result, mask, 0.9 ,1, THRESH_BINARY);
        mask.convertTo(mask, CV_8UC1);
        mask = mask*255;
        //imshow("Threshold image", mask);
        //waitKey(0);
        vector<vector<Point>> contours;
        findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
        Scalar color(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255));
        for(size_t i=0; i < contours.size(); i++) {
            Rect region = boundingRect(contours[i]);

            Mat temp = result(region);
            Point matchLoc;
            minMaxLoc( temp, NULL, NULL, NULL, &matchLoc, Mat());

            Point p1 ( region.x + matchLoc.x, region.y + matchLoc.y ); // Top left
            Point p2 ( (region.x + matchLoc.x + templ.cols), region.y + matchLoc.y ); // Top right
            Point p3 ( (region.x + matchLoc.x + templ.cols), (region.y + matchLoc.y + templ.rows) ); // Down right
            Point p4 ( region.x + matchLoc.x, (region.y + matchLoc.y + templ.rows) ); // Down left

            // Add the 4 points to a matrix structure
            Mat coordinates = (Mat_<double>(3,4) << p1.x, p2.x, p3.x, p4.x,\
                                                    p1.y, p2.y, p3.y, p4.y,\
                                                    1   , 1  ,  1   , 1    );

            // Apply a new inverse tranformation matrix
            Point2f pt(image1.cols/2., image1.rows/2.);
            Mat r = getRotationMatrix2D(pt, -(step_angle*(j+1)), 1.0);
            Mat result2 = r * coordinates;

            // Retrieve the new coordinates from the tranformed matrix
            Point p1_back, p2_back, p3_back, p4_back;
            p1_back.x=(int)result2.at<double>(0,0);
            p1_back.y=(int)result2.at<double>(1,0);

            p2_back.x=(int)result2.at<double>(0,1);
            p2_back.y=(int)result2.at<double>(1,1);

            p3_back.x=(int)result2.at<double>(0,2);
            p3_back.y=(int)result2.at<double>(1,2);

            p4_back.x=(int)result2.at<double>(0,3);
            p4_back.y=(int)result2.at<double>(1,3);

            // Draw a rotated rectangle by lines, using the reverse warped points
            line(img_display, p1_back, p2_back, color, 2);
            line(img_display, p2_back, p3_back, color, 2);
            line(img_display, p3_back, p4_back, color, 2);
            line(img_display, p4_back, p1_back, color, 2);
        }
    }
    imshow("Detection result", img_display);
}
*/
