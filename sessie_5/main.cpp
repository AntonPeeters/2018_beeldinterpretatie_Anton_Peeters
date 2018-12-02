#include <iostream>
#include <opencv2/opencv.hpp>
#include "ml.h"

using namespace std;
using namespace cv;
using namespace ml;

Mat img;

static void onMouse( int event, int x, int y, int, void* param)
{
    vector<Point>* pixels = (vector<Point>*)param;

    Point seed = Point(x,y);

    if( event == EVENT_LBUTTONDOWN )
    {
        pixels->push_back(seed);
        cerr << "Added: " << seed << "\n";
    }
    if( event == EVENT_RBUTTONDOWN )
    {
        if(pixels->size() != 0)
            pixels->pop_back();
            cerr << "Removed last point \n";
    }
    if( event == EVENT_MBUTTONDOWN )
    {
        for(int i = 0; i< pixels->size();i++)
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

    namedWindow("image", 0);

    imshow("image", image);

    /// Creating a vector of points for the user to select pixels on screen representing a strawberry
    vector<Point> strawberry;
    setMouseCallback("image", onMouse, &strawberry );
    waitKey(0);

    cerr << "select some negative examples.\n";

    /// Creating a vector of points for the user to select pixels on screen representing a not-strawberry
    vector<Point> background;
    setMouseCallback("image", onMouse, &background );
    waitKey(0);

    Mat img_hsv = Mat(image.clone());

    /// Removing the Green channel from image because strawberries are typically red
    //for (int i = 0; i < image.rows; i++) img_hsv.row(i).reshape(1, image.cols).col(1).setTo(Scalar(0));

    img_hsv -= Scalar(0,255,0);

    /// Adding some blur to cancel out mistakes
    GaussianBlur(img_hsv, img_hsv, Size(5,5),0);

    /// Converting to HSV
    cvtColor(img_hsv,img_hsv,COLOR_BGR2HSV);

    Mat trainingDataForeground(strawberry.size(), 3, CV_32FC1);
    Mat labels_fg = Mat::ones(strawberry.size(), 1, CV_32SC1);

    for(int i = 0; i < strawberry.size(); i++)
    {
        Vec3b descriptor = img_hsv.at<Vec3b>(strawberry[i].y, strawberry[i].x);
        trainingDataForeground.at<float>(i,0) = descriptor[0];
        trainingDataForeground.at<float>(i,1) = descriptor[1];
        trainingDataForeground.at<float>(i,2) = descriptor[2];
    }

    Mat trainingDataBackground(background.size(), 3, CV_32FC1);
    Mat labels_bg = Mat::zeros(background.size(), 1, CV_32SC1);

    for(int i = 0; i < background.size(); i++)
    {
        Vec3b descriptor = img_hsv.at<Vec3b>(background[i].y, background[i].x);
        trainingDataBackground.at<float>(i,0) = descriptor[0];
        trainingDataBackground.at<float>(i,1) = descriptor[1];
        trainingDataBackground.at<float>(i,2) = descriptor[2];
    }

    Mat matTrainFeatures , matTrainLabels;
    vconcat(trainingDataForeground,trainingDataBackground,matTrainFeatures);
    vconcat(labels_fg,labels_bg,matTrainLabels);

    /// Training using the KNearest algorithm
    Ptr<TrainData> trainingData = TrainData::create(matTrainFeatures, ROW_SAMPLE, matTrainLabels);
    Ptr<KNearest> kclassifier = KNearest::create();

    kclassifier->setIsClassifier(true);
    kclassifier->setAlgorithmType(KNearest::Types::BRUTE_FORCE);
    kclassifier->setDefaultK(3);
    kclassifier->train(trainingData);

    /// Training using the NormalBayes algorithm
    Ptr<NormalBayesClassifier> normalBayes = NormalBayesClassifier::create();
    normalBayes->train(matTrainFeatures, ROW_SAMPLE, matTrainLabels);

    /// Training using the Support Vector Machine algorithm
    Ptr<SVM> svm = SVM::create();
    svm->setType(SVM::C_SVC);
    svm->setKernel(SVM::LINEAR);
    svm->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, 100, 1e-6));
    svm->train(matTrainFeatures, ROW_SAMPLE, matTrainLabels);


    /// Executing the different classifiers and creating a mask to show on screen
    Mat labels_knn, labels_bayes, labels_svm;
    Mat mask_knn = Mat::zeros(image.rows, image.cols, CV_8UC1);
    Mat mask_bayes = Mat::zeros(image.rows, image.cols, CV_8UC1);
    Mat mask_svm = Mat::zeros(image.rows, image.cols, CV_8UC1);

    for(int r = 0; r < image.rows; r++) {
        for(int c = 0; c < image.cols; c++) {
            Vec3b pixelvalue = img_hsv.at<Vec3b>(r,c);
            Mat data_test(1, 3, CV_32FC1);
            data_test.at<float>(0,0) = pixelvalue[0];
            data_test.at<float>(0,1) = pixelvalue[1];
            data_test.at<float>(0,2) = pixelvalue[2];

            kclassifier->findNearest(data_test, kclassifier->getDefaultK(), labels_knn);
            normalBayes->predict(data_test, labels_bayes);
            svm->predict(data_test, labels_svm);

            mask_knn.at<uchar>(r,c) = labels_knn.at<float>(0,0);
            mask_bayes.at<uchar>(r,c) = labels_bayes.at<int>(0,0);
            mask_svm.at<uchar>(r,c) = labels_svm.at<float>(0,0);
        }
    }

    imshow("knn", mask_knn*255);
    imshow("bayes", mask_bayes*255);
    imshow("svm", mask_svm*255);

    waitKey(0);
    return 0;
}
