#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, const char** argv)
{
    /// Adding a little help option and command line parser input
    CommandLineParser parser(argc, argv,
        "{ help h usage ? || show this message }"
        "{ color c|| (required) path to image }"
        "{ bimodal b|| (required) path to image }"
    );

    if (parser.has("help")){
        parser.printMessage();
        return 0;
    }

    /// Collect data from arguments
    string image_bimodal_loc(parser.get<string>("bimodal"));
    string image_color_loc(parser.get<string>("color"));
    if (image_bimodal_loc.empty() || image_color_loc.empty()){
        cerr << "error while reading your images, check if you put the correct path.";
        parser.printMessage();
        return -1;
    }

    /// Loading the images and showing it on screen
    Mat imageColor= imread(image_color_loc);
    Mat imageBimodal= imread(image_bimodal_loc);
    if(imageColor.empty() || imageBimodal.empty()) {
        cerr << "error while loading your images, check if you put the correct path.";
        return -1;
    }

    imshow("OrigineelColor", imageColor);
    waitKey(0);

    /// Segmenting all the skin pixels from loaded picture
    vector<Mat> channels;
    split(imageColor,channels);
    Mat BLUE = channels[0];
    Mat GREEN = channels[1];
    Mat RED = channels[2];
    Mat canvas = Mat::zeros(imageColor.rows, imageColor.cols, CV_8UC1);
    Mat canvas2 = Mat::zeros(imageColor.rows, imageColor.cols, CV_8UC1);

    /// Alternative 1
    for(int row = 0; row < imageColor.rows; row++)
    {
        for(int column = 0; column < imageColor.cols; column++) {
            if((RED.at<uchar>(row, column)>95) && (GREEN.at<uchar>(row, column)>40) && (BLUE.at<uchar>(row, column)>20)\
            && ((max(RED.at<uchar>(row, column),max(GREEN.at<uchar>(row, column),BLUE.at<uchar>(row, column))) -\
            min(RED.at<uchar>(row, column),min(GREEN.at<uchar>(row, column),BLUE.at<uchar>(row, column))))>15) &&\
            (abs(RED.at<uchar>(row, column)-GREEN.at<uchar>(row, column))>15) && (RED.at<uchar>(row, column)>GREEN.at<uchar>(row, column)) &&\
            (RED.at<uchar>(row, column)>BLUE.at<uchar>(row, column))) {
                canvas.at<uchar>(row, column) = 255;
            }
        }
    }
    imshow("Canvas", canvas);
    waitKey(0);

    /// Alternative 2
    canvas2 = (RED>95) & (GREEN>40) & (BLUE>20) & ((max(RED,max(GREEN,BLUE)) - min(RED,min(GREEN,BLUE)))>15) & (abs(RED-GREEN)>15) & (RED>GREEN) & (RED>BLUE);
    canvas2 = canvas2*255;
    imshow("Canvas2", canvas2);
    waitKey(0);

    /// Merging the original image with the skin segmented image
    Mat finaal(imageColor.rows, imageColor.cols, CV_8UC3);
    Mat PBLUE = channels[0] & canvas2;
    Mat PGREEN = channels[1] & canvas2;
    Mat PRED = channels[2] & canvas2;

    Mat in[] = {PBLUE, PGREEN, PRED};
    int from_to[] = {0,0,1,1,2,2};
    mixChannels(in, 3, &finaal, 1, from_to, 3);

    imshow("Masker", finaal);
    waitKey(0);

    /// Resizing the bimodal image
    resize(imageBimodal,imageBimodal, Size(imageBimodal.cols/2, imageBimodal.rows/2));
    imshow("OrigineelBimodal", imageBimodal);
    waitKey(0);

    /// Converting the bimodal image to a grey image
    Mat grey_bimodal;
    cvtColor(imageBimodal, grey_bimodal, COLOR_RGB2GRAY);
    imshow("grey",grey_bimodal);
    waitKey(0);

    /// Thresholding the grey image
    Mat maskaa;
    threshold(grey_bimodal, maskaa, 0 ,255, THRESH_OTSU | THRESH_BINARY);
    imshow("grey threshold", maskaa);
    waitKey(0);

    /// Equalizing the grey image
    Mat grey_bimodal_equalized;
    equalizeHist(grey_bimodal.clone(),grey_bimodal_equalized);
    imshow("equalized", grey_bimodal_equalized);
    waitKey(0);

    /// Thresholding the equalized image
    Mat maska;
    threshold(grey_bimodal_equalized, maska, 0 ,255, THRESH_OTSU | THRESH_BINARY);
    imshow("equalized threshold", maska);
    waitKey(0);

    /// CLAHE on the grey image
    Mat result_CLAHE;
    Ptr<CLAHE> clahe_pointer = createCLAHE();
    clahe_pointer->setTilesGridSize(Size(30,30));
    clahe_pointer->setClipLimit(1);
    clahe_pointer->apply(grey_bimodal.clone(), result_CLAHE);
    imshow("clahe", result_CLAHE);
    waitKey(0);

    /// Thresholding the CLAHE image
    Mat mask;
    threshold(result_CLAHE, mask, 0 ,255, THRESH_OTSU | THRESH_BINARY);
    imshow("clahe threshold", mask);
    waitKey(0);

    return 0;
}
