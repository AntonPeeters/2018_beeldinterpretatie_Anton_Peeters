#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <string>

using namespace std;
using namespace cv;

string key = "10110011";
int bits;
void imageEncoding(Mat img1, Mat img2);
void textEncoding(Mat img1, ifstream& file);

/// Converting a decimal into a binary number
string intToBin(int decimal) {
    int binary = 0;
    int remainder, i = 1;
    while (decimal!=0)
    {
        remainder = decimal%2;
        decimal /= 2;
        binary += remainder*i;
        i *= 10;
    }

    /// Converting to string to make concatenating easier
    string bin = to_string(binary);

    /// Adding zeros in front so it is easier to strip
    while(bin.size()<8)
        bin = "0" + bin;
    return bin;
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

/// Merging the two bitgroups together
int mergeBGR(string n, string o) {

    ///Dropping the last "bits" bits
    n = n.substr(0,n.size()-(8-bits));
    o = o.substr(0,o.size()-bits);

    /// Adding them together
    string total = n+o;
    return stoi(total);
}

bool isBitOne(char ch, int pos) {
	ch = ch >> pos; /// Shifts to the right
	ch = ch ^ key[pos]; /// Encrypting the letters with a key
	if(ch & 1)
		return true;
	return false;
}

int main(int argc, const char** argv)
{
    /// Adding a little help option and command line parser input
    CommandLineParser parser(argc, argv,
        "{ help h usage ? || show this message }"
        "{ bits b|| (required) amount of bits for hidden image storage}"
        "{ image1 i1|| (required) path to image }"
        "{ image_text it|| (required) path to image or text file }"
    );

    if (parser.has("help")){
        parser.printMessage();
        return 0;
    }

    /// Collect data from arguments
    bits = 8-parser.get<int>("bits");
    if(bits < 1 || bits > 7) {
        cerr << "error while reading your bits, check if between 1 and 7.";
        parser.printMessage();
        return -1;
    }
    string image_image1_loc(parser.get<string>("image1"));
    string image_image_text_loc(parser.get<string>("image_text"));
    if (image_image1_loc.empty() || image_image_text_loc.empty()){
        cerr << "error while reading your images, check if you put the correct path.";
        parser.printMessage();
        return -1;
    }

    /// Loading the image and showing it on screen
    Mat img1 = imread(image_image1_loc);
        if(img1.empty()) {
            cerr << "error while loading your image, check if you put the correct path.";
            return -1;
        }
    Mat img2 = imread(image_image_text_loc);
    if(img2.empty()) {
        ifstream file(image_image_text_loc);
            if(!file.is_open()) {
                cerr << "error while loading your image/text file, check if you put the correct path.";
            return -1;
            }
            else {
                /// Showing the images on screen
                imshow("1", img1 );
                waitKey(0);

                textEncoding(img1, file);
            }
    }
    else {
        /// Check if the image that will be hidden fits in the original image
        if(img2.rows > img1.rows || img2.cols > img1.cols) {
            cerr << "Image 1 size is lower than image 2 size!";
            return -1;
        }

        /// Showing the images on screen
        imshow("1", img1 );
        imshow("2", img2 );
        waitKey(0);

        imageEncoding(img1, img2);
    }

    return 0;
}

/// We will hide an image into another image by taking the x amount of MSB (given by the user) of the hidden image,
/// and storing them in the original image as LSB. This way the original image will not alter as much given it are only the LSB that are changed.
/// A higher amount of bits will result in a better quality hidden image, at the cost of being less hidden in the original image.
/// A lower amount of bits will result in a lower quality hidden image, but it will be hidden better.
void imageEncoding(Mat img1, Mat img2) {
    /// A black default image
    Mat canvas = Mat::zeros(Size(img1.size()), CV_8UC3);

    /// Splitting the images into the BGR channels
    vector<Mat> channels1, channels2, channels3;
    split(img1,channels1);
    split(img2,channels2);
    split(canvas,channels3);

    /// Triple for-loop that iterates over each pixel and its color channel
    for(int row = 0; row < img1.rows; row++) {
        for(int column = 0; column < img1.cols; column++) {
            for(int i = 0; i < 3; i++) {
                int bgr;
                string bgr1, bgr2;
                bgr1 = intToBin(channels1[i].at<uchar>(row, column));
                bgr2 = intToBin(0); /// Take black as default
                if(row < img2.rows && column < img2.cols) /// If-test to check if still in range of second image
                    bgr2 = intToBin(channels2[i].at<uchar>(row, column));
                bgr = mergeBGR(bgr1,bgr2);
                int pixelvalue = binToInt(bgr);
                channels3[i].at<uchar>(row, column) = pixelvalue;
            }
        }
    }

    /// Merging the channels into an image
    merge(channels3, canvas);

    /// Saving the image and showing it on screen
    imwrite( "encrypted.png", canvas); /// Important to save as an ".png" because ".jpg" alters the pixelvalues
    imshow("Encrypted Image", canvas );
    waitKey(0);
}

/// We hide text into an image by taking one char (8 bits), and storing them in the LSB of an image pixel (BGR).
/// If we do it this way the change of the color of pixel will not be noticeable.
void textEncoding(Mat img1, ifstream& file) {
	char ch;
	file.get(ch);
	int bit_count = 0; /// The bit we are working on.
	bool file_end = false; /// Check if file ended
	bool encoded = false; /// Check if encoded successfully

	/// Triple for-loop that iterates over each pixel and its color channel
	for(int row=0; row < img1.rows; row++) {
		for(int column=0; column < img1.cols; column++) {
			for(int i=0; i < 3; i++) {
				Vec3b pixel = img1.at<Vec3b>(row,column);

                /// If 1 -> set LSB to 1
                /// If 0 -> set LSB to 0
				if(isBitOne(ch,7-bit_count))
					pixel.val[i] |= 1;
				else
					pixel.val[i] &= 0;

				img1.at<Vec3b>(row,column) = pixel;

				/// Next bit
				bit_count++;

				/// Our message is encoded
				if(file_end && bit_count == 8) {
					encoded  = true;
					goto OUT;
				}

				/// End of one char
				if(bit_count == 8) {
					bit_count = 0;
					cout<<ch;
					file.get(ch);

					/// When end of file
					if(file.eof()) {
						file_end = true;
						ch = '\0';
					}
				}
			}
		}
	}
    OUT:;

	if(!encoded) {
		cerr << "Message too big. Try with larger image.\n";
		exit(-1);
	}

	/// Saving the image and showing it on screen
    imwrite("encryptedText.png",img1);
    imshow("Encrypted Text", img1 );
    waitKey(0);
}
