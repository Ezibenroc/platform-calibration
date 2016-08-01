Analysis of Pont-to-point experiments of MPI calls
==================================================

```r
opts_chunk$set(cache=FALSE,dpi=300,echo=FALSE)
```


If needed, you should install the right packages (plyr, ggplot2, and
knitr) with the install.packages command.

```
## Loading required package: plyr
```

```
## Loading required package: ggplot2
```

```
## Loading required package: XML
```

```
## Loading required package: methods
```
Load XML config file and .csv resulting files from the MPI execution


```
## Warning in read.table(file = file, header = header, sep = sep,
## quote = quote, : incomplete final line found by readTableHeader on
## 'testplatform_breakpoints'
```

MPI_Send timing
---------------

Timings for this experiment are taken from a ping-pong experiment, used to determine os.

We determine the piecewiese regression based on information taken from the regression file pointed in the XML configuration file


```
##        Limit         Name LimitInf
## 1       1420        Small        0
## 2      65536       Medium     1420
## 3     131072 Asynchronous    65536
## 4     400000     Detached   131072
## 5 2147483647        Large   400000
```
Display the regression factors to help tuning.

```
## [1] "----- Small-----"
## 
## Call:
## lm(formula = Duration ~ Size, data = d[d$group == bp[bp$Limit == 
##     lim, ]$Name, ])
## 
## Residuals:
##        Min         1Q     Median         3Q        Max 
## -8.576e-07 -1.958e-07 -2.000e-09  1.761e-07  6.609e-06 
## 
## Coefficients:
##              Estimate Std. Error t value Pr(>|t|)    
## (Intercept) 2.707e-06  3.210e-09  843.31   <2e-16 ***
## Size        2.473e-10  8.065e-12   30.66   <2e-16 ***
## ---
## Signif. codes:  0 '***' 0.001 '**' 0.01 '*' 0.05 '.' 0.1 ' ' 1
## 
## Residual standard error: 3.241e-07 on 14523 degrees of freedom
## Multiple R-squared:  0.06078,	Adjusted R-squared:  0.06072 
## F-statistic: 939.9 on 1 and 14523 DF,  p-value: < 2.2e-16
## 
## [1] "----- Medium-----"
## 
## Call:
## lm(formula = Duration ~ Size, data = d[d$group == bp[bp$Limit == 
##     lim, ]$Name, ])
## 
## Residuals:
##        Min         1Q     Median         3Q        Max 
## -1.264e-06 -2.623e-07 -2.060e-08  2.257e-07  9.285e-06 
## 
## Coefficients:
##              Estimate Std. Error t value Pr(>|t|)    
## (Intercept) 3.975e-06  8.757e-09   454.0   <2e-16 ***
## Size        7.238e-11  4.083e-13   177.3   <2e-16 ***
## ---
## Signif. codes:  0 '***' 0.001 '**' 0.01 '*' 0.05 '.' 0.1 ' ' 1
## 
## Residual standard error: 4.928e-07 on 6721 degrees of freedom
## Multiple R-squared:  0.8238,	Adjusted R-squared:  0.8238 
## F-statistic: 3.143e+04 on 1 and 6721 DF,  p-value: < 2.2e-16
## 
## [1] "----- Asynchronous-----"
## 
## Call:
## lm(formula = Duration ~ Size, data = d[d$group == bp[bp$Limit == 
##     lim, ]$Name, ])
## 
## Residuals:
##        Min         1Q     Median         3Q        Max 
## -1.930e-06 -4.889e-07 -6.840e-08  4.260e-07  7.273e-06 
## 
## Coefficients:
##              Estimate Std. Error t value Pr(>|t|)    
## (Intercept) 7.588e-06  9.862e-08   76.94   <2e-16 ***
## Size        6.771e-11  1.064e-12   63.63   <2e-16 ***
## ---
## Signif. codes:  0 '***' 0.001 '**' 0.01 '*' 0.05 '.' 0.1 ' ' 1
## 
## Residual standard error: 7.093e-07 on 1318 degrees of freedom
## Multiple R-squared:  0.7544,	Adjusted R-squared:  0.7542 
## F-statistic:  4048 on 1 and 1318 DF,  p-value: < 2.2e-16
```
Visual representation of the computed data, to visualize correctness of the computed value.

The black line representing the regression should be very close to the values, and should drop to 0 when communications use the rendez-vous algorithm (Large messages, with a size > eager_threshold).

If they are not, tune the breakpoints in order to match more closely to your implementation. Thresholds for eager and detached messages depend on the library and the hardware used. Consult the documentation of your library on how to display this information if you can't visually determine it (For Ethernet network we saw values of 65536, while IB networks had values of 12288 or 17408 depending on the implementation)


```
## Saving 7 x 7 in image
```

```
## Warning: Removed 1 rows containing missing values (geom_vline).
```

```
## Warning: Removed 1 rows containing missing values (geom_text).
```

```
## Warning: Removed 1 rows containing missing values (geom_vline).
```

```
## Warning: Removed 1 rows containing missing values (geom_text).
```

![plot of chunk unnamed-chunk-6](figure/unnamed-chunk-6-1.png)
MPI_Isend timing
---------------

As they may differ from Send times, check this and call it ois, to inject proper timings later.


Display the regression factors to help tuning

```
## [1] "----- Small-----"
## 
## Call:
## lm(formula = Duration ~ Size, data = d[d$group == bp[bp$Limit == 
##     lim, ]$Name, ])
## 
## Residuals:
##        Min         1Q     Median         3Q        Max 
## -7.582e-07 -1.726e-07 -6.310e-09  1.630e-07  1.387e-06 
## 
## Coefficients:
##              Estimate Std. Error t value Pr(>|t|)    
## (Intercept) 2.774e-06  2.443e-09 1135.56   <2e-16 ***
## Size        2.163e-10  6.140e-12   35.23   <2e-16 ***
## ---
## Signif. codes:  0 '***' 0.001 '**' 0.01 '*' 0.05 '.' 0.1 ' ' 1
## 
## Residual standard error: 2.467e-07 on 14526 degrees of freedom
## Multiple R-squared:  0.07873,	Adjusted R-squared:  0.07866 
## F-statistic:  1241 on 1 and 14526 DF,  p-value: < 2.2e-16
## 
## [1] "----- Medium-----"
## 
## Call:
## lm(formula = Duration ~ Size, data = d[d$group == bp[bp$Limit == 
##     lim, ]$Name, ])
## 
## Residuals:
##        Min         1Q     Median         3Q        Max 
## -1.183e-06 -2.404e-07 -5.100e-09  2.273e-07  1.040e-05 
## 
## Coefficients:
##              Estimate Std. Error t value Pr(>|t|)    
## (Intercept) 4.037e-06  6.886e-09   586.2   <2e-16 ***
## Size        6.922e-11  3.210e-13   215.7   <2e-16 ***
## ---
## Signif. codes:  0 '***' 0.001 '**' 0.01 '*' 0.05 '.' 0.1 ' ' 1
## 
## Residual standard error: 3.874e-07 on 6720 degrees of freedom
## Multiple R-squared:  0.8738,	Adjusted R-squared:  0.8737 
## F-statistic: 4.651e+04 on 1 and 6720 DF,  p-value: < 2.2e-16
## 
## [1] "----- Asynchronous-----"
## 
## Call:
## lm(formula = Duration ~ Size, data = d[d$group == bp[bp$Limit == 
##     lim, ]$Name, ])
## 
## Residuals:
##        Min         1Q     Median         3Q        Max 
## -1.798e-06 -4.276e-07 -2.410e-08  4.013e-07  4.151e-06 
## 
## Coefficients:
##              Estimate Std. Error t value Pr(>|t|)    
## (Intercept) 5.776e-06  9.097e-08   63.49   <2e-16 ***
## Size        8.194e-11  9.816e-13   83.47   <2e-16 ***
## ---
## Signif. codes:  0 '***' 0.001 '**' 0.01 '*' 0.05 '.' 0.1 ' ' 1
## 
## Residual standard error: 6.543e-07 on 1318 degrees of freedom
## Multiple R-squared:  0.8409,	Adjusted R-squared:  0.8408 
## F-statistic:  6968 on 1 and 1318 DF,  p-value: < 2.2e-16
```
Visual representation of the computed data, to visualize correctness of the computed value

```
## Saving 7 x 7 in image
```

```
## Warning: Removed 1 rows containing missing values (geom_vline).
```

```
## Warning: Removed 1 rows containing missing values (geom_text).
```

```
## Warning: Removed 1 rows containing missing values (geom_vline).
```

```
## Warning: Removed 1 rows containing missing values (geom_text).
```

![plot of chunk unnamed-chunk-9](figure/unnamed-chunk-9-1.png)

MPI_Recv timing
---------------

Timings are used to determine or. This experiment waits for a potentially eager message to arrive before launching the recv for small message size, eliminating waiting times as much as possible.


Display the regression factors to help tuning

```
## [1] "----- Small-----"
## 
## Call:
## lm(formula = Duration ~ Size, data = d[d$group == bp[bp$Limit == 
##     lim, ]$Name, ])
## 
## Residuals:
##        Min         1Q     Median         3Q        Max 
## -4.275e-06 -2.404e-06 -3.605e-07  1.400e-06  1.335e-05 
## 
## Coefficients:
##              Estimate Std. Error t value Pr(>|t|)    
## (Intercept) 5.243e-06  3.099e-08  169.18   <2e-16 ***
## Size        2.059e-09  7.789e-11   26.44   <2e-16 ***
## ---
## Signif. codes:  0 '***' 0.001 '**' 0.01 '*' 0.05 '.' 0.1 ' ' 1
## 
## Residual standard error: 3.129e-06 on 14528 degrees of freedom
## Multiple R-squared:  0.04591,	Adjusted R-squared:  0.04585 
## F-statistic: 699.1 on 1 and 14528 DF,  p-value: < 2.2e-16
## 
## [1] "----- Medium-----"
## 
## Call:
## lm(formula = Duration ~ Size, data = d[d$group == bp[bp$Limit == 
##     lim, ]$Name, ])
## 
## Residuals:
##        Min         1Q     Median         3Q        Max 
## -9.122e-06 -4.919e-06  3.510e-07  4.306e-06  2.592e-05 
## 
## Coefficients:
##              Estimate Std. Error t value Pr(>|t|)    
## (Intercept) 1.136e-05  9.469e-08  119.97   <2e-16 ***
## Size        2.833e-10  4.415e-12   64.18   <2e-16 ***
## ---
## Signif. codes:  0 '***' 0.001 '**' 0.01 '*' 0.05 '.' 0.1 ' ' 1
## 
## Residual standard error: 5.328e-06 on 6721 degrees of freedom
## Multiple R-squared:   0.38,	Adjusted R-squared:  0.3799 
## F-statistic:  4119 on 1 and 6721 DF,  p-value: < 2.2e-16
## 
## [1] "----- Asynchronous-----"
## 
## Call:
## lm(formula = Duration ~ Size, data = d[d$group == bp[bp$Limit == 
##     lim, ]$Name, ])
## 
## Residuals:
##        Min         1Q     Median         3Q        Max 
## -1.469e-05 -8.253e-06  3.458e-06  5.517e-06  2.148e-05 
## 
## Coefficients:
##              Estimate Std. Error t value Pr(>|t|)    
## (Intercept) 7.369e-06  1.107e-06   6.656 4.11e-11 ***
## Size        3.322e-10  1.195e-11  27.807  < 2e-16 ***
## ---
## Signif. codes:  0 '***' 0.001 '**' 0.01 '*' 0.05 '.' 0.1 ' ' 1
## 
## Residual standard error: 7.963e-06 on 1318 degrees of freedom
## Multiple R-squared:  0.3698,	Adjusted R-squared:  0.3693 
## F-statistic: 773.3 on 1 and 1318 DF,  p-value: < 2.2e-16
```
Visual representation of the computed data, to visualize correctness of the computed value

```
## Saving 7 x 7 in image
```

```
## Warning: Removed 1 rows containing missing values (geom_vline).
```

```
## Warning: Removed 1 rows containing missing values (geom_text).
```

```
## Warning: Removed 1 rows containing missing values (geom_vline).
```

```
## Warning: Removed 1 rows containing missing values (geom_text).
```

![plot of chunk unnamed-chunk-12](figure/unnamed-chunk-12-1.png)

Pingpong timing
---------------

pingpong = 2or+2transfer for small messages that are sent
  asynchronously.  For large sizes, communications are synchronous,
  hence we have pingpong = 2transfer.


```
## [1] "----- Small-----"
## 
## Call:
## lm(formula = Duration ~ Size, data = d[d$group == bp[bp$Limit == 
##     lim, ]$Name, ])
## 
## Residuals:
##        Min         1Q     Median         3Q        Max 
## -5.649e-06 -1.290e-06 -5.890e-07  1.590e-07  1.057e-04 
## 
## Coefficients:
##              Estimate Std. Error t value Pr(>|t|)    
## (Intercept) 1.999e-05  2.829e-08  706.53   <2e-16 ***
## Size        6.084e-09  7.111e-11   85.56   <2e-16 ***
## ---
## Signif. codes:  0 '***' 0.001 '**' 0.01 '*' 0.05 '.' 0.1 ' ' 1
## 
## Residual standard error: 3.193e-06 on 18148 degrees of freedom
## Multiple R-squared:  0.2874,	Adjusted R-squared:  0.2874 
## F-statistic:  7321 on 1 and 18148 DF,  p-value: < 2.2e-16
## 
## [1] "----- Medium-----"
## 
## Call:
## lm(formula = Duration ~ Size, data = d[d$group == bp[bp$Limit == 
##     lim, ]$Name, ])
## 
## Residuals:
##        Min         1Q     Median         3Q        Max 
## -1.110e-05 -4.253e-06 -2.128e-06  8.600e-08  1.478e-04 
## 
## Coefficients:
##              Estimate Std. Error t value Pr(>|t|)    
## (Intercept) 3.474e-05  1.372e-07   253.1   <2e-16 ***
## Size        1.889e-09  6.398e-12   295.3   <2e-16 ***
## ---
## Signif. codes:  0 '***' 0.001 '**' 0.01 '*' 0.05 '.' 0.1 ' ' 1
## 
## Residual standard error: 8.632e-06 on 8398 degrees of freedom
## Multiple R-squared:  0.9122,	Adjusted R-squared:  0.9121 
## F-statistic: 8.72e+04 on 1 and 8398 DF,  p-value: < 2.2e-16
## 
## [1] "----- Asynchronous-----"
## 
## Call:
## lm(formula = Duration ~ Size, data = d[d$group == bp[bp$Limit == 
##     lim, ]$Name, ])
## 
## Residuals:
##        Min         1Q     Median         3Q        Max 
## -1.099e-05 -5.231e-06 -3.405e-06  3.520e-07  1.042e-04 
## 
## Coefficients:
##              Estimate Std. Error t value Pr(>|t|)    
## (Intercept) 3.840e-05  1.180e-06   32.54   <2e-16 ***
## Size        1.784e-09  1.273e-11  140.11   <2e-16 ***
## ---
## Signif. codes:  0 '***' 0.001 '**' 0.01 '*' 0.05 '.' 0.1 ' ' 1
## 
## Residual standard error: 9.491e-06 on 1648 degrees of freedom
## Multiple R-squared:  0.9226,	Adjusted R-squared:  0.9225 
## F-statistic: 1.963e+04 on 1 and 1648 DF,  p-value: < 2.2e-16
## 
## [1] "----- Detached-----"
## 
## Call:
## lm(formula = Duration ~ Size, data = d[d$group == bp[bp$Limit == 
##     lim, ]$Name, ])
## 
## Residuals:
##        Min         1Q     Median         3Q        Max 
## -2.026e-05 -9.460e-06 -6.350e-06 -1.290e-06  3.606e-04 
## 
## Coefficients:
##              Estimate Std. Error t value Pr(>|t|)    
## (Intercept) 8.892e-05  1.454e-06   61.17   <2e-16 ***
## Size        1.719e-09  6.575e-12  261.38   <2e-16 ***
## ---
## Signif. codes:  0 '***' 0.001 '**' 0.01 '*' 0.05 '.' 0.1 ' ' 1
## 
## Residual standard error: 2.214e-05 on 2748 degrees of freedom
## Multiple R-squared:  0.9613,	Adjusted R-squared:  0.9613 
## F-statistic: 6.832e+04 on 1 and 2748 DF,  p-value: < 2.2e-16
## 
## [1] "----- Large-----"
## 
## Call:
## lm(formula = Duration ~ Size, data = d[d$group == bp[bp$Limit == 
##     lim, ]$Name, ])
## 
## Residuals:
##        Min         1Q     Median         3Q        Max 
## -2.456e-05 -1.433e-05 -1.071e-05 -4.960e-06  4.328e-04 
## 
## Coefficients:
##              Estimate Std. Error t value Pr(>|t|)    
## (Intercept) 8.951e-05  3.757e-06   23.82   <2e-16 ***
## Size        1.715e-09  5.420e-12  316.37   <2e-16 ***
## ---
## Signif. codes:  0 '***' 0.001 '**' 0.01 '*' 0.05 '.' 0.1 ' ' 1
## 
## Residual standard error: 4.043e-05 on 2148 degrees of freedom
## Multiple R-squared:  0.979,	Adjusted R-squared:  0.979 
## F-statistic: 1.001e+05 on 1 and 2148 DF,  p-value: < 2.2e-16
```

```
## Saving 7 x 7 in image
```

```
## Warning: Removed 1 rows containing missing values (geom_vline).
```

```
## Warning: Removed 1 rows containing missing values (geom_text).
```

```
## Warning: Removed 1 rows containing missing values (geom_vline).
```

```
## Warning: Removed 1 rows containing missing values (geom_text).
```

![plot of chunk unnamed-chunk-13](figure/unnamed-chunk-13-1.png)![plot of chunk unnamed-chunk-13](figure/unnamed-chunk-13-2.png)

Print results in Simgrid's xml format

  
MPI_Wtime timing
---------------

We made a run with 10 millions calls to MPI\_Wtime and we want to know the time of one call


Time for one MPI_Wtime call

```
## [1] 3.098699e-08
```
MPI_Iprobe timing
----------------
We made 1000 runs of pingpong with pollling on MPI\_Iprobe. Compute the Duration of such a call, and check whether its time is related to the size of the message

![plot of chunk unnamed-chunk-17](figure/unnamed-chunk-17-1.png)
Time for one MPI_Iprobe call

```
## [1] 2.339273e-07
```

MPI_Test timing
---------------

![plot of chunk unnamed-chunk-19](figure/unnamed-chunk-19-1.png)
Time for one MPI_Test call

```
## [1] 2.280189e-07
```


Result of calibration.
---------------

The following snippet of XML has to be included at the beginning of your platformfile. Please report to the SimGrid mailing list any bug with the calibration or the generated platform file.


```
## <config id="General">
##  <prop id="smpi/os" value="0:2.70683864220123e-06:2.47269722619783e-10;1420:3.97545175056929e-06:7.23786118047894e-11;65536:7.58804087458976e-06:6.77088730090102e-11;131072:0:0;4e+05:0:0"/>
##  <prop id="smpi/ois" value="0:2.7741781272553e-06:2.1631688187721e-10;1420:4.03664653739977e-06:6.92199520863807e-11;65536:5.77600529533672e-06:8.19394370464538e-11;131072:2.46349531812281e-06:1.48757168395357e-13;4e+05:2.43434499482972e-06:3.03855696558919e-13"/>
##  <prop id="smpi/or" value="0:5.24254663627748e-06:2.05940556065388e-09;1420:1.13604897462265e-05:2.8332717225738e-10;65536:7.36920463371884e-06:3.32186380684382e-10;131072:0:0;4e+05:0:0"/>
##  <prop id="smpi/bw_factor" value="0:0.124230489840844;1420:0.311345781078732;65536:0.344336843153613;131072:0.290935589385778;4e+05:0.291581596908999"/>
##  <prop id="smpi/lat_factor" value="0:0.737322772282333;1420:1.16891951785859;65536:1.55145779857452;131072:4.44599650869728;4e+05:4.47543800174862"/>
##  <prop id="smpi/async_small_thres" value="65536"/>
##  <prop id="smpi/send_is_detached_thres" value="320000"/>
##  <prop id="smpi/wtime" value="3.098699e-08"/>
##  <prop id="smpi/iprobe" value="2.33927297668038e-07"/>
##  <prop id="smpi/test" value="2.28018867924528e-07"/>
## </config>
```

```
## [1] "Results written in sirocco.xml"
```

```
## [1] "sirocco_output.xml"
```

