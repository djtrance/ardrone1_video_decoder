Once you could read binary video stream, you can decode it. The overall video stream structure is as follow.
figure1

The stream starts with picture header and it contains GOB information at the same time so the GOB header for GOB#0 is omitted. After GOB#0 data, some number of zeros are inserted so that the next GOB starts at the first bit of new element of unsigned 32-bit array.


The content of picture header is explained in Developers Guide as follow. Note that we are only considering the case of UVLC.



Figure.2 Picture Header

Picture Start Code (PSC) in picture header and GOB Start Code (GOBSC) in each of GOB header shares the same coherent numbering, so PSC is always 0000 0000 0000 0000 1000 00 and GOBSC are 0000 0000 0000 0000 1000 01 ~ 0000 0000 0000 0000 1000 14. Picture Format, Picture Resolution, and Picture Types are always fixed to the same numbers, so we can leave them as they are. 5-bit Picture Quantizer is the q value of quantization explained here. Developers Guide says "The PQUANT code is a 5-bits-long word. The quantizer’s reference for the picture that range from 1 to 30." But this is not correct. Since after running my own video decoder for hours, I saw PQUANT value is always fixed to 31 (However automatic bitrate adjustment was always turned off, so those who are going to use autobitrate might have to care.) I am guessing that since the video is sent in the best quality, (the picture of Yoshi is the example) so PQUANT must be its lowest, that is 1. So actual PQUANT value from AR.Drone is (32-PQUANT). Picture Frame Number is incremental counter and set to zero every time AR.Drone is turned on.

After picture header comes the content of the first GOB, GOB#0. Again, the same information as GOB header is included in picture header, so GOB #0 header is omitted. GOB content is composed of 20 macroblocks. 


Figure.3 GOB data

Each macroblock has header and actual image data stored as blocks Y0, Y1, Y2, Y3, U0 and V0.



Figure.4 Macroblock Content

MBC indicates if there is non-zero value block in this macroblock. If this bit is not set, you can move onto the next macroblock. *IMPORTANT* Developer Guide says MBC being 1 means there are some blocks that has non-zero values, but actually it's opposite, MBC=0 means there is macro block content to be decoded. MBDES indicates which block has AC value (DC is the bias term of Discrete Cosine Transform and AC is the other coefficients.), and also indicates if the block is encoded with differential quantization, but it is not currently (and probably never, since AR.Drone 2.0 is out now) implemented we can ignore this and following MBDIFF bits. 

MBDES bits are bit ordered (not sure if this term is technically correct, so see the following example), so intuitively reverse ordered than bits we decoded so far. For example we received such macroblock header as, 010001011, the first 0 is MBC meaning that there is block data to be decoded, and following 10001011 means, reading from right digit, Y0 and Y1 block has AC value (1 1), Y2 does not (0), Y3 does (1), U0 and V0 do not have AC values (0 0), differential quantization is not used (0), and the first bit is always 1.

Figure.5 Macroblock Description Code

If bit7 should be 1, then  you have to read 2 more MBDIFF bits before decoding YUV blocks.
Each YUV block can be decoded in the manner described in my previous post. Again the sample binary stream of Developer Guide is not coherent, so take care when you test your decoder.

One more thing you have to know and is not written in Developer Guide is that each GOB header (for GOB#1~14 ) starts in new 32 bit array, so there are many zeros inserted between the last bit of V0 block content and the next GOB header.

GOB header contains Start Code (22 bits) and Quantizer value (5 bits) in the same way as picture header does.

The simple C video decoder I coded by modifying official API is attached below. For displaying the image decoded, I used OpenCV, so you need to install OpenCV in to your system, then compile it with 

$ gcc testDrone_video.c 'pkg-config --cflags --libs opencv'

and run giving the IP address of AR.Drone as command line argument.

$ ./a.out 192.168.1.1

You will see RGB colour (3 channel) and grayscale (1 channel) images.



Sample Decoding

testDrone_video.tar.gz

One thing to be reminded is that, for YUV->RGB conversion, we used the equation found in Wikipedia and 


Figure.6 YUV to RGB conversion
http://en.wikipedia.org/wiki/YUV#Y.27UV444_to_RGB888_conversion

The offset for chroma value is 128, but that was not the case for AR.Drone and we used 80. You can change this value defined in video.h file.

By integrating OpenCV, we can use many cool image processing algorithm. 
Below is an ORB feature detection implementation.





