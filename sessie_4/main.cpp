#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

Mat img;
Mat templ;

void orb();
void akaze();
void brisk();

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
    img= imread(image_image_loc);
    if(img.empty()) {
        cerr << "error while loading your images, check if you put the correct path.";
        return -1;
    }

    /// Loading the template image
    templ= imread(image_templ_loc);
    if(templ.empty()) {
        cerr << "error while loading your images, check if you put the correct path.";
        return -1;
    }

    orb();
    akaze();
    brisk();

    return 0;
}

void orb() {

    vector<KeyPoint> keypoints_object, keypoints_scene;
    Mat descriptors_object, descriptors_scene;

    /// Detect keypoints using ORB
    Ptr<ORB> orb = ORB::create();
    orb->detect(templ, keypoints_object);
    orb->detect(img, keypoints_scene);

    /// Drawing keypoints on object and scene
    Mat img_keypoints_object, img_keypoints_scene;
    drawKeypoints(templ, keypoints_object, img_keypoints_object, Scalar::all(-1), DrawMatchesFlags::DEFAULT);
    drawKeypoints(img, keypoints_scene, img_keypoints_scene, Scalar::all(-1), DrawMatchesFlags::DEFAULT);

    /// Placing the object and scene next to each other
    Mat canvas(Size(img_keypoints_object.cols+img_keypoints_scene.cols, img_keypoints_scene.rows), CV_8UC3);
    Mat left(canvas, Rect(0, 0, img_keypoints_object.cols, img_keypoints_object.rows));
    img_keypoints_object.copyTo(left);
    Mat right(canvas, Rect(img_keypoints_object.cols, 0, img_keypoints_scene.cols, img_keypoints_scene.rows));
    img_keypoints_scene.copyTo(right);

    imshow("Matches ORB", canvas);
    waitKey(0);

    /// Calculating the descriptors
    Ptr<DescriptorExtractor> extractor = ORB::create();
    extractor->compute(templ, keypoints_object, descriptors_object );
    extractor->compute(img, keypoints_scene, descriptors_scene );

    /// Matching descriptor vectors using BruteForce matcher
    BFMatcher matcher(NORM_L2);
    vector< DMatch > matches;
    matcher.match( descriptors_object, descriptors_scene, matches );

    double max_dist = 0; double min_dist = 100;

    /// Quick calculation of max and min distances between keypoints
    for( int i = 0; i < descriptors_object.rows; i++ )
    {
        double dist = matches[i].distance;
        if( dist < min_dist ) min_dist = dist;
        if( dist > max_dist ) max_dist = dist;
    }

    vector< DMatch > good_matches;
    /// Filter matches using the Lowe's ratio test
    for( int i = 0; i < descriptors_object.rows; i++ )
    {
        if( matches[i].distance <= 3*min_dist )
        {
            good_matches.push_back( matches[i]);
        }
    }

    Mat img_matches;
    drawMatches( templ, keypoints_object, img, keypoints_scene, good_matches, img_matches, Scalar::all(-1),
                 Scalar::all(-1), std::vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );

    std::vector<Point2f> obj;
    std::vector<Point2f> scene;
    int aantal = 0;
    for( size_t i = 0; i < good_matches.size(); i++ )
    {
        /// Get the keypoints from the good matches
        obj.push_back( keypoints_object[ good_matches[i].queryIdx ].pt );
        scene.push_back( keypoints_scene[ good_matches[i].trainIdx ].pt );
        aantal += 1;
    }
    printf("aantal goede matches voor ORB = %d\n", aantal);

    /// Needs at least 4 matches to work
    if (aantal <= 3) return;

    Mat H = findHomography( obj, scene, RANSAC );

    /// Get the corners from the object ( the object to be "detected" )
    std::vector<Point2f> obj_corners(4);
    obj_corners[0] = Point2f(0, 0);
    obj_corners[1] = Point2f( (float)templ.cols, 0 );
    obj_corners[2] = Point2f( (float)templ.cols, (float)templ.rows );
    obj_corners[3] = Point2f( 0, (float)templ.rows );
    std::vector<Point2f> scene_corners(4);
    perspectiveTransform( obj_corners, scene_corners, H);
    /// Draw lines between the corners (the mapped object in the scene )
    line( img_matches, scene_corners[0] + Point2f((float)templ.cols, 0),
          scene_corners[1] + Point2f((float)templ.cols, 0), Scalar(0, 255, 0), 4 );
    line( img_matches, scene_corners[1] + Point2f((float)templ.cols, 0),
          scene_corners[2] + Point2f((float)templ.cols, 0), Scalar( 0, 255, 0), 4 );
    line( img_matches, scene_corners[2] + Point2f((float)templ.cols, 0),
          scene_corners[3] + Point2f((float)templ.cols, 0), Scalar( 0, 255, 0), 4 );
    line( img_matches, scene_corners[3] + Point2f((float)templ.cols, 0),
          scene_corners[0] + Point2f((float)templ.cols, 0), Scalar( 0, 255, 0), 4 );
    /// Show detected matches
    imshow("Good Matches & Object detection ORB", img_matches );
    waitKey(0);
}

void akaze() {

    vector<KeyPoint> keypoints_object, keypoints_scene;
    Mat descriptors_object, descriptors_scene;

    /// Detect keypoints using AKAZE
    Ptr<AKAZE> akaze = AKAZE::create();
    akaze->detect(templ, keypoints_object);
    akaze->detect(img, keypoints_scene);

    /// Drawing keypoints on object and scene
    Mat img_keypoints_object, img_keypoints_scene;
    drawKeypoints(templ, keypoints_object, img_keypoints_object, Scalar::all(-1), DrawMatchesFlags::DEFAULT);
    drawKeypoints(img, keypoints_scene, img_keypoints_scene, Scalar::all(-1), DrawMatchesFlags::DEFAULT);

    /// Placing the object and scene next to each other
    Mat canvas(Size(img_keypoints_object.cols+img_keypoints_scene.cols, img_keypoints_scene.rows), CV_8UC3);
    Mat left(canvas, Rect(0, 0, img_keypoints_object.cols, img_keypoints_object.rows));
    img_keypoints_object.copyTo(left);
    Mat right(canvas, Rect(img_keypoints_object.cols, 0, img_keypoints_scene.cols, img_keypoints_scene.rows));
    img_keypoints_scene.copyTo(right);

    imshow("Matches AKAZE", canvas);
    waitKey(0);

    /// Calculating the descriptors
    Ptr<DescriptorExtractor> extractor = AKAZE::create();
    extractor->compute(templ, keypoints_object, descriptors_object );
    extractor->compute(img, keypoints_scene, descriptors_scene );

    /// Matching descriptor vectors using BruteForce matcher
    BFMatcher matcher(NORM_L2);
    vector< DMatch > matches;
    matcher.match( descriptors_object, descriptors_scene, matches );

    double max_dist = 0; double min_dist = 100;

    /// Quick calculation of max and min distances between keypoints
    for( int i = 0; i < descriptors_object.rows; i++ )
    {
        double dist = matches[i].distance;
        if( dist < min_dist ) min_dist = dist;
        if( dist > max_dist ) max_dist = dist;
    }

    vector< DMatch > good_matches;
    /// Filter matches using the Lowe's ratio test
    for( int i = 0; i < descriptors_object.rows; i++ )
    {
        if( matches[i].distance <= 3*min_dist )
        {
            good_matches.push_back( matches[i]);
        }
    }

    /// Draw matches
    Mat img_matches;
    drawMatches( templ, keypoints_object, img, keypoints_scene, good_matches, img_matches, Scalar::all(-1),
                 Scalar::all(-1), std::vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
    /// Localize the object
    std::vector<Point2f> obj;
    std::vector<Point2f> scene;
    int aantal = 0;
    for( size_t i = 0; i < good_matches.size(); i++ )
    {
        /// Get the keypoints from the good matches
        obj.push_back( keypoints_object[ good_matches[i].queryIdx ].pt );
        scene.push_back( keypoints_scene[ good_matches[i].trainIdx ].pt );
        aantal+=1;
    }
    printf("aantal goede matches voor AKAZE = %d\n", aantal);

    /// Needs at least 4 matches to work
    if (aantal <= 3) return;

    Mat H = findHomography( obj, scene, RANSAC );

    /// Get the corners from the object ( the object to be "detected" )
    std::vector<Point2f> obj_corners(4);
    obj_corners[0] = Point2f(0, 0);
    obj_corners[1] = Point2f( (float)templ.cols, 0 );
    obj_corners[2] = Point2f( (float)templ.cols, (float)templ.rows );
    obj_corners[3] = Point2f( 0, (float)templ.rows );
    std::vector<Point2f> scene_corners(4);
    perspectiveTransform( obj_corners, scene_corners, H);
    /// Draw lines between the corners (the mapped object in the scene )
    line( img_matches, scene_corners[0] + Point2f((float)templ.cols, 0),
          scene_corners[1] + Point2f((float)templ.cols, 0), Scalar(0, 255, 0), 4 );
    line( img_matches, scene_corners[1] + Point2f((float)templ.cols, 0),
          scene_corners[2] + Point2f((float)templ.cols, 0), Scalar( 0, 255, 0), 4 );
    line( img_matches, scene_corners[2] + Point2f((float)templ.cols, 0),
          scene_corners[3] + Point2f((float)templ.cols, 0), Scalar( 0, 255, 0), 4 );
    line( img_matches, scene_corners[3] + Point2f((float)templ.cols, 0),
          scene_corners[0] + Point2f((float)templ.cols, 0), Scalar( 0, 255, 0), 4 );
    /// Show detected matches
    imshow("Good Matches & Object detection AKAZE", img_matches );
    waitKey(0);
}

void brisk() {

    vector<KeyPoint> keypoints_object, keypoints_scene;
    Mat descriptors_object, descriptors_scene;

    /// Detect keypoints using BRISK
    Ptr<BRISK> brisk = BRISK::create();
    brisk->detect(templ, keypoints_object);
    brisk->detect(img, keypoints_scene);

    /// Drawing keypoints on object and scene
    Mat img_keypoints_object, img_keypoints_scene;
    drawKeypoints(templ, keypoints_object, img_keypoints_object, Scalar::all(-1), DrawMatchesFlags::DEFAULT);
    drawKeypoints(img, keypoints_scene, img_keypoints_scene, Scalar::all(-1), DrawMatchesFlags::DEFAULT);

    /// Placing the object and scene next to each other
    Mat canvas(Size(img_keypoints_object.cols+img_keypoints_scene.cols, img_keypoints_scene.rows), CV_8UC3);
    Mat left(canvas, Rect(0, 0, img_keypoints_object.cols, img_keypoints_object.rows));
    img_keypoints_object.copyTo(left);
    Mat right(canvas, Rect(img_keypoints_object.cols, 0, img_keypoints_scene.cols, img_keypoints_scene.rows));
    img_keypoints_scene.copyTo(right);

    imshow("Matches BRISK", canvas);
    waitKey(0);

    /// Calculating the descriptors
    Ptr<DescriptorExtractor> extractor = BRISK::create();
    extractor->compute(templ, keypoints_object, descriptors_object );
    extractor->compute(img, keypoints_scene, descriptors_scene );

    /// Matching descriptor vectors using BruteForce matcher
    BFMatcher matcher(NORM_L2);
    vector< DMatch > matches;
    matcher.match( descriptors_object, descriptors_scene, matches );

    double max_dist = 0; double min_dist = 100;

    /// Quick calculation of max and min distances between keypoints
    for( int i = 0; i < descriptors_object.rows; i++ )
    {
        double dist = matches[i].distance;
        if( dist < min_dist ) min_dist = dist;
        if( dist > max_dist ) max_dist = dist;
    }

    vector< DMatch > good_matches;
    /// Filter matches using the Lowe's ratio test
    for( int i = 0; i < descriptors_object.rows; i++ )
    {
        if( matches[i].distance <= 3*min_dist )
        {
            good_matches.push_back( matches[i]);
        }
    }

    /// Draw matches
    Mat img_matches;
    drawMatches( templ, keypoints_object, img, keypoints_scene, good_matches, img_matches, Scalar::all(-1),
                 Scalar::all(-1), std::vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
    /// Localize the object
    std::vector<Point2f> obj;
    std::vector<Point2f> scene;
    int aantal = 0;
    for( size_t i = 0; i < good_matches.size(); i++ )
    {
        /// Get the keypoints from the good matches
        obj.push_back( keypoints_object[ good_matches[i].queryIdx ].pt );
        scene.push_back( keypoints_scene[ good_matches[i].trainIdx ].pt );
        aantal+=1;
    }
    printf("aantal goede matches voor BRISK = %d\n", aantal);

    /// Needs at least 4 matches to work
    if (aantal <= 3) return;

    Mat H = findHomography( obj, scene, RANSAC );

    /// Get the corners from the object ( the object to be "detected" )
    std::vector<Point2f> obj_corners(4);
    obj_corners[0] = Point2f(0, 0);
    obj_corners[1] = Point2f( (float)templ.cols, 0 );
    obj_corners[2] = Point2f( (float)templ.cols, (float)templ.rows );
    obj_corners[3] = Point2f( 0, (float)templ.rows );
    std::vector<Point2f> scene_corners(4);
    perspectiveTransform( obj_corners, scene_corners, H);
    /// Draw lines between the corners (the mapped object in the scene )
    line( img_matches, scene_corners[0] + Point2f((float)templ.cols, 0),
          scene_corners[1] + Point2f((float)templ.cols, 0), Scalar(0, 255, 0), 4 );
    line( img_matches, scene_corners[1] + Point2f((float)templ.cols, 0),
          scene_corners[2] + Point2f((float)templ.cols, 0), Scalar( 0, 255, 0), 4 );
    line( img_matches, scene_corners[2] + Point2f((float)templ.cols, 0),
          scene_corners[3] + Point2f((float)templ.cols, 0), Scalar( 0, 255, 0), 4 );
    line( img_matches, scene_corners[3] + Point2f((float)templ.cols, 0),
          scene_corners[0] + Point2f((float)templ.cols, 0), Scalar( 0, 255, 0), 4 );
    /// Show detected matches
    imshow("Good Matches & Object detection BRISK", img_matches );
    waitKey(0);
}
