#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

Mat img;

static void onMouse( int event, int x, int y, int, void* param)
{
    vector<Point>* pixels = (vector<Point>*)param;

    if( event == EVENT_LBUTTONDOWN )
    {
        Point seed = Point(x,y);
        pixels->push_back(seed);
        cerr << seed << "\n";
    }
    if( event == EVENT_RBUTTONDOWN )
    {
        if(pixels->size() != 0)
            pixels->pop_back();
    }
    if( event == EVENT_MBUTTONDOWN )
    {
        for(size_t i = 0; i< pixels->size();i++)
        {
                cerr << pixels->at(i) << " ";
        }
        cerr << "\n";
    }
}

int main(int argc, const char** argv)
{
    /// Adding a little help option and command line parser input
    CommandLineParser parser(argc, argv,
        "{ help h usage ? || show this message }"
        "{ image i1|| (required) path to image }"
    );

    if (parser.has("help")){
        parser.printMessage();
        return 0;
    }

    /// Collect data from arguments
    string image_image_loc(parser.get<string>("image"));
    if (image_image_loc.empty()){
        cerr << "error while reading your images, check if you put the correct path.";
        parser.printMessage();
        return -1;
    }

    /// Loading the image and showing it on screen
    img= imread(image_image_loc);
    if(img.empty()) {
        cerr << "error while loading your images, check if you put the correct path.";
        return -1;
    }

    Mat image;
    img.copyTo(image);
    GaussianBlur(image, image, Size(5,5),0,0);

    namedWindow( "image", 0 );

    imshow("image", image);

    vector<Point> strawberry;
    setMouseCallback("image", onMouse, &strawberry );

    waitKey(0);

    vector<Point> background;
    setMouseCallback("image", onMouse, &background );

    Mat img_hsv;
    cvtColor(image,img_hsv,COLOR_BGR2HSV);

    Mat trainingDataForeground(strawberry.size(), 3, CV_32FC1);
    Mat labels_fg = Mat::ones(strawberry.size(), 1, CV_32FC1);

    for(int i = 0; i<strawberry.size();i++)
    {
        Vec3b descriptor = img_hsv.at<Vec3b>(strawberry[i].y, strawberry[i].x);
        trainingDataForeground.at<float>(i,0) = descriptor[0];
        trainingDataForeground.at<float>(i,1) = descriptor[1];
        trainingDataForeground.at<float>(i,2) = descriptor[2];
    }

    Mat trainingDataBackground(background.size(), 3, CV_32FC1);
    Mat labels_bg = Mat::ones(background.size(), 1, CV_32FC1);

    for(int i = 0; i<background.size();i++)
    {
        Vec3b descriptor = img_hsv.at<Vec3b>(background[i].y, background[i].x);
        trainingDataBackground.at<float>(i,0) = descriptor[0];
        trainingDataBackground.at<float>(i,1) = descriptor[1];
        trainingDataBackground.at<float>(i,2) = descriptor[2];
    }

    Mat trainingData , labels;
    vconcat(trainingDataForeground,trainingDataBackground,trainingData);
    vconcat(labels_fg,labels_bg,labels);

    Ptr<KNearest> kNN = KNearest::create();
    Ptr<TrainData> trainDatakNN = TrainData::create(trainingData, ROW_SAMPLE, labels);


    waitKey(0);
    return 0;
}
