#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

string key = "01100110";
int bits;
void imageDecoding(Mat img1);
void textDecoding(Mat img1);

/// Converting a decimal into a binary number
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

/// Converting a binary number into a decimal
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

/// Extracting the last bits of the encrypted image which are from the hidden image
int extractBGR(int n) {
    int power = pow(10,8-bits);
    int remainder = n%power;
    return pow(10,bits)*remainder;
}

bool isBitSet(char ch, int pos, int keypos) {
	ch = ch >> pos;
	ch = ch ^ key[keypos];
	if(ch & 1)
		return true;
	return false;
}

int main(int argc, const char** argv)
{
    /// Adding a little help option and command line parser input
    CommandLineParser parser(argc, argv,
        "{ help h usage ? || show this message }"
        "{ bits b || (required) amount of bits for image storage}"
        "{ image1 i1|| (required) path to image }"
        "{ image_text it || (required) specify if image or text was hidden}"
    );

    if (parser.has("help")){
        parser.printMessage();
        return 0;
    }

    /// Collect data from arguments
    bits = parser.get<int>("bits");
    if(bits < 1 || bits > 7) {
        cerr << "error while reading your bits, check if between 1 and 7.";
        parser.printMessage();
        return -1;
    }
    string image_image1_loc(parser.get<string>("image1"));
    if (image_image1_loc.empty()){
        cerr << "error while reading your image, check if you put the correct path.";
        parser.printMessage();
        return -1;
    }

    /// Loading the image and showing it on screen
    Mat img1 = imread(image_image1_loc);
    if(img1.empty()) {
        cerr << "error while loading your image1, check if you put the correct path.";
        return -1;
    }

    /// Showing encrypted image on screen
    imshow("1", img1 );
    waitKey(0);

    if(parser.get<string>("image_text") == "image")
        imageDecoding(img1);
    else if(parser.get<string>("image_text") == "text")
        textDecoding(img1);
    else {
        cerr << "Put 'image' for an image or 'text' for a text file.";
        parser.printMessage();
        return -1;
    }

    return 0;
}

void imageDecoding(Mat img1){
    /// A black default image
    Mat canvas = Mat::zeros(Size(img1.size()), CV_8UC3);

    /// Width and height of the hidden image
    int original_w, original_h;

    /// Splitting the images into the BGR channels
    vector<Mat> channels1, channels2;
    split(img1,channels1);
    split(canvas,channels2);

    /// Triple for-loop that irritates over each pixel and its color channel
    for(int row = 0; row < img1.rows; row++) {
        for(int column = 0; column < img1.cols; column++) {
            for(int i = 0; i < 3; i++) {
                int bgr, bgr1;
                bgr1 = intToBin(channels1[i].at<uchar>(row, column));
                bgr = extractBGR(bgr1);
                int pixelvalue = binToInt(bgr);
                channels2[i].at<uchar>(row, column) = pixelvalue;
            }

            /// If-test to check if hidden image is smaller than the original, so we can crop the black bars if necessary
            if(channels2[0].at<uchar>(row, column)!=0 || channels2[1].at<uchar>(row, column)!=0 || channels2[2].at<uchar>(row, column)!=0) {
                original_h = row+1;
                original_w = column+1;
            }
        }
    }
    /// Merging the channels into an image
    merge(channels2, canvas);

    /// Cropping the image
    Mat original = canvas(Rect(0,0,original_w,original_h));

    /// Saving the image and showing it on screen
    imwrite("decrypted.jpg", original);
    imshow("Decrypted image", original);
    waitKey(0);
}

void textDecoding(Mat img1) {
	char ch=0;
	int bit_count = 0; /// The bit we are working on.

    /// Triple for-loop that irritates over each pixel and its color channel
	for(int row=0; row < img1.rows; row++) {
		for(int col=0; col < img1.cols; col++) {
			for(int color=0; color < 3; color++) {
				Vec3b pixel = img1.at<Vec3b>(Point(row,col));

				/// Test if bit is set
				if(isBitSet(pixel.val[color],0,bit_count))
					ch |= 1;

				/// Next bit
				bit_count++;

				/// End of one char
				if(bit_count == 8) {

					/// End of the message
					if(ch == '\0')
						return;

					bit_count = 0;
					cout <<ch;
					ch = 0;
				}
				else {
					ch = ch << 1;
				}

			}
		}
	}
}
