/* Compute/draw the Mandelbrot set using TSGL and OpenMP
 *
 * Joel Adams, for CS 374, Fall 2023, Calvin University.
 * Alex Miller, CS 374, 29 Nov 2023, Calvin University
 */

#include <cstdio>           // C-style I/O
#include <complex>          // complex<T>
#include <cstdlib>          // abs()
#include <tsgl.h>           // CartesianCanvas, etc.
#include <omp.h>
#include <stdlib.h>   // atoi()
using namespace tsgl;

const int THRESHOLD = 500;  // our Mandelbrot 'escape' threshold

int doMandelbrotCalc(long double x, long double y, int MAX_REPS=THRESHOLD);
void drawMandelbrotPoint(CartesianCanvas& canvas,
		          long double x, long double y, 
			  int reps, int MAX_REPS=THRESHOLD);

int main(int argc, char* argv[]) {
    const int  WINDOW_HEIGHT = 800;
    const int  WINDOW_WIDTH  = 1200;

    if (argc > 1){
        omp_set_num_threads( atoi(argv[1]) );
    } else {
        printf("Usage: ./mandel {number of threads}\n");
        exit(0);
    }

    printf("\nComputing and drawing the Mandelbrot Set...\n");
    const double startTime = omp_get_wtime();

    CartesianCanvas canvas(-1, -1, WINDOW_WIDTH, WINDOW_HEIGHT, 
                           -2, -1.125, 1, 1.125, 
                           "Mandelbrot Set (Calvin U)", GRAY);
    canvas.start();

    unsigned    winHeight = canvas.getWindowHeight()-1,  // TSGL bug ?
	        winWidth  = canvas.getWindowWidth();
    long double startX    = canvas.getMinX(),
		deltaX    = canvas.getPixelWidth(),
		startY    = canvas.getMinY(),
		deltaY    = canvas.getPixelHeight();

    #pragma omp parallel for
    for (unsigned row = 0; row < winHeight; ++row) {
       long double y = startY + row * deltaY;	      
       for (unsigned col = 0; col < winWidth; ++col) {  
          long double x = startX + col * deltaX;
          int iterations = doMandelbrotCalc(x, y);
	  drawMandelbrotPoint(canvas, x, y, iterations);
       }
    }

    const double runTime = omp_get_wtime() - startTime;
    printf("\nTotal runtime: %f\n", runTime);

    // pause so the program doesn't terminate
    printf("\nMandelbrot Set completed.\n"
	   "\nPress ESC or click the window's close-box to quit...\n\n");
    canvas.wait();
}


/* perform the Mandelbrot calculation for a given x,y point
 * @param: x, a long double
 * @param: y, a long double
 * @param: MAX_REPS, an int
 * Precondition: MAX_REPS is a value, such that we assume
 *                calculations that iterate more than 
 *                that many times never converge.
 * Postcondition: count == MAX_REPS ||
 *                count == the number of Mandelbrot iterations
 *                          required for (x,y) to converge.
 * @return: count
 */
int doMandelbrotCalc(long double x, long double y, int MAX_REPS) {
    std::complex<long double> originalComplex(x, y);
    std::complex<long double> comp(x, y);
    int count = 0;
    while (std::abs(comp) < 2.0 && count < MAX_REPS) {
        comp = comp * comp + originalComplex;
        ++count;
    }
    return count;
}

/* draw a given (x,y) point, color based on whether or not
 *   (x,y) is within or outside the Mandelbrot set
 * @param: canvas, a TSGL CartesianCanvase
 * @param: x, a long double
 * @param: y, a long double
 * @param: reps, an int
 * @param: MAX_REPS, an int
 * Precondition: reps == doMandelbrotCalcFor(x,y)
 *            && canvas models a slice through the complex plane.
 * Postcondition: the (x,y) pixel on canvas has been shaded [using reps] 
 *                 for inclusion/exclusion in the Mandelbrot set
 *                 depending on how reps relates to MAX_REPS.
 */
void drawMandelbrotPoint(CartesianCanvas& canvas,
		          long double x, long double y, 
			  int reps, int MAX_REPS) {
   CartesianBackground *bg  = canvas.getBackground();

   ColorInt figureColor(238,204,10);              // Gold
   ColorInt surroundColor(137,27,47);             // Maroon
/*
   ColorInt figureColor(0,0,0);                   // Alternative
   ColorInt surroundColor(255,255,0);             //  colors
*/
   if (reps >= MAX_REPS) {
       bg->drawPixel(x, y, figureColor);
   } else {
       bg->drawPixel(x, y, surroundColor);
   }
}

