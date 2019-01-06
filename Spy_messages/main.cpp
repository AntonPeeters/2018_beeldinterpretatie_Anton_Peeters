#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int intToBin(int decimal) {
    int binary = 0;
    int remainder, i = 1;
    while (decimal!=0)
    {
        remainder = decimal%2;
        decimal /= 2;
        binary += remainder*i;
        i *= 10;
    }
    return binary;
}

int binToInt(int binary) {
    int decimal = 0, i = 0, remainder;
    while (binary!=0)
    {
        remainder = binary%10;
        binary /= 10;
        decimal += remainder*pow(2,i);
        ++i;
    }
    return decimal;
}

//a/10^n
int mergeBGR(int n, int o) {
    int times = 1;
    while (times <= (o/10000))
        times *= 10;
    return (n/10000)*times + (o/10000);
}

int main(int argc, const char** argv)
{
    /// Adding a little help option and command line parser input
    CommandLineParser parser(argc, argv,
        "{ help h usage ? || show this message }"
        "{ image1 i1|| (required) path to image }"
        "{ image2 i2|| (required) path to image }"
    );

    if (parser.has("help")){
        parser.printMessage();
        return 0;
    }

    /// Collect data from arguments
    string image_image1_loc(parser.get<string>("image1"));
    string image_image2_loc(parser.get<string>("image2"));
    if (image_image1_loc.empty() || image_image1_loc.empty()){
        cerr << "error while reading your images, check if you put the correct path.";
        parser.printMessage();
        return -1;
    }

    /// Loading the image and showing it on screen
    Mat img1 = imread(image_image1_loc);
    Mat img2 = imread(image_image2_loc);
    if(img1.empty()) {
        cerr << "error while loading your image1, check if you put the correct path.";
        return -1;
    }
    if(img2.empty()) {
        cerr << "error while loading your image2, check if you put the correct path.";
        return -1;
    }
    if(img2.rows > img1.rows || img2.cols > img1.cols) {
        cerr << "Image 1 size is lower than image 2 size!";
        return -1;
    }
    /*
    imshow("1", img1 );
    imshow("2", img2 );
    waitKey(0);
*/
    Mat canvas = Mat::zeros(Size(250,250), CV_8UC3);
    vector<Mat> channels1, channels2, channels3;
    split(img1,channels1);
    split(img2,channels2);
    split(canvas,channels3);
    for(int row = 0; row < img1.rows; row++) {
        for(int column = 0; column < img1.cols; column++) {
            for(int i = 0; i < 3; i++) {
                int bgr[3], bgr1[3], bgr2[3];
                bgr1[i] = intToBin(channels1[i].at<uchar>(row, column));
                bgr2[i] = intToBin(0);
                if(row < img2.rows && column < img2.cols)
                    bgr2[i] = intToBin(channels2[i].at<uchar>(row, column));
                bgr[i] = mergeBGR(bgr1[i],bgr2[i]);
                int iets = binToInt(bgr[i]);
                channels3[i].at<uchar>(row, column) = iets;
            }
            if(row == 271)
                 cout<<column<<"/"<<img1.cols<<" col\n";
        }
        //cout<<row<<"/"<<img2.rows<<" row\n";
    }
    merge(channels3, canvas);
    imshow("Jones", canvas );
    waitKey(0);
    return 0;
}
