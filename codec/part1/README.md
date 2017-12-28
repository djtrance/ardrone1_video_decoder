Receiving navdata is also the first step of receiving video stream, but decoding the video stream requires more effort, because there are a lot of things we need to know to decode the image but are not written in the reference manual, or because the manual is wrong. We referred SDK reference 1.7.

The video stream is sent in a jpg-like format. The picture is first split in 15 groups of blocks (GOB). Each of GOB consisting of 16x320 pixel parts is then split into 20 16x16 Macroblolcks. Figure.1 illustrates this. Each 16x16 Macroblock is then covered into 8x8 blocks using YUV 4:2:0 conversion, that is, 4 8x8 blocks of grayscale, and scaled-down Cb/Cr chroma components. (See Figure.2).



Each block is now 8x8 integer matrix and this matrix is converted with Discrete Cosine Transform. After spending hours on this, we are sure that one of  DCT equation (or modified one) described in this section http://en.wikipedia.org/wiki/YUV#Y.27UV444_to_RGB888_conversion is used but not sure exactly which one is, but we will explain decoding process later so it will not be a problem. DCT decomposes series of values to series of coefficient of linear combination of cosine curves, and so pushes values towards upper row and left column leaving many elements lying middle to bottom right to zero. The values of most top rows or most left columns are called DC values and the rest are AC values. Figure.3 summarizes the decomposition process.

After DCT, the block is quantified by the following matrix. Each element of the block is divided by the corresponding elements separately.

Then, the elements of the matrix are reordered in so-called zig-zag ordering. (See Figure.3) To make use of lots of zeros and to decrease the data size, then the series of elements are encoded with Run Length Encoding and Huffman Coding. The detail of these decode process are explained well in Developer Guide so will not be explained here, but we point out some errors there.


And it is worth pointing that the video data stream always start with non-zero 10-digit shortened 16 bits [2 bytes] signed integer. (That is "-57" in the example above.)

To receive video stream from AR.Drone, simply send one UDP packet to its port 5555. The video stream is stored as 32-bit array and sent over network with little endian. So declare unsigned 32-bit array as follow,

uint32_t bufferedVideoStream[10000];

and receive video stream with recvfrom function. The following code will display binary video stream as explained in Developer Guide.

int i, j;
for (j=0;j<receivedvideosizeinbyte/4;j++) {
  for (i=31; i>=0; i--) {
    printf("%d", (bufferedVideoStream[j]&(1<<i))!=0);
  }
}

If you dump the network packet sent from AR.Drone with software such as Wireshark, and save a UDP in C array style ( you can download sample dumped file from API forum ) , you will see the first few values are such [in hex].

0x43, 0x82, 0x00, 0x00, 0x0d, 0x0a, 0x00, 0xe0, 0x0f, 0xe2, 0xf4, 0xcd, 0xce, 0x92 ...

AR.Drone sends data as series of 32-bit word, and each 8 bits are aligned reverse ordering (little endien). (I am just a university student and not professional programmer and also not good at English, so these terms may be wrong, and if you find a mistake, please let me know.)
To interpret these values as binary video stream as explained in AR.Drone Developer Guide, convert the address of array, bufferedVideoStream to 8-bit array display in the order each 4 elements are reverse ordered.

uint8_t* long2char = (uint8_t*)bufferedVideoStream;
printf("%x %x %x %x", long2char[3], long2char[2], long2char[1], long2char[0]);


The result would look like, 


       00         00         82         43
0000 0000  0000 0000  1000 0010  0100 0011


       e0         00         03         3c
1110 0000  0000 0000  0000 0011  0011 1100

       6b         f2         dd         a9
0110 1011  1111 0010  1101 1101  1010 1001

Once you could read those binary streams, then you can start decoding from reading picture header.
