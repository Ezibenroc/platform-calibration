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
## 4     450000     Detached   131072
## 5 2147483647        Large   450000
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
## -3.284e-07 -1.158e-07 -1.000e-10  7.310e-08  6.775e-06 
## 
## Coefficients:
##              Estimate Std. Error t value Pr(>|t|)    
## (Intercept) 1.723e-06  1.922e-09   896.6   <2e-16 ***
## Size        2.806e-10  4.837e-12    58.0   <2e-16 ***
## ---
## Signif. codes:  0 '***' 0.001 '**' 0.01 '*' 0.05 '.' 0.1 ' ' 1
## 
## Residual standard error: 1.945e-07 on 14580 degrees of freedom
## Multiple R-squared:  0.1875,	Adjusted R-squared:  0.1874 
## F-statistic:  3364 on 1 and 14580 DF,  p-value: < 2.2e-16
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
## -2.668e-07 -9.030e-08 -1.600e-09  5.860e-08  6.320e-06 
## 
## Coefficients:
##              Estimate Std. Error t value Pr(>|t|)    
## (Intercept) 1.792e-06  1.272e-09 1408.34   <2e-16 ***
## Size        2.610e-10  3.198e-12   81.59   <2e-16 ***
## ---
## Signif. codes:  0 '***' 0.001 '**' 0.01 '*' 0.05 '.' 0.1 ' ' 1
## 
## Residual standard error: 1.287e-07 on 14589 degrees of freedom
## Multiple R-squared:  0.3133,	Adjusted R-squared:  0.3133 
## F-statistic:  6657 on 1 and 14589 DF,  p-value: < 2.2e-16
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
## -3.468e-06 -6.060e-07 -3.960e-07 -1.380e-07  4.217e-05 
## 
## Coefficients:
##              Estimate Std. Error t value Pr(>|t|)    
## (Intercept) 3.278e-06  1.975e-08  166.04   <2e-16 ***
## Size        3.417e-09  4.962e-11   68.86   <2e-16 ***
## ---
## Signif. codes:  0 '***' 0.001 '**' 0.01 '*' 0.05 '.' 0.1 ' ' 1
## 
## Residual standard error: 1.995e-06 on 14546 degrees of freedom
## Multiple R-squared:  0.2458,	Adjusted R-squared:  0.2458 
## F-statistic:  4742 on 1 and 14546 DF,  p-value: < 2.2e-16
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
## -6.508e-05 -1.964e-05 -1.379e-05  1.045e-05  1.899e-04 
## 
## Coefficients:
##              Estimate Std. Error t value Pr(>|t|)    
## (Intercept) 4.543e-05  3.015e-07   150.7   <2e-16 ***
## Size        3.849e-08  7.578e-10    50.8   <2e-16 ***
## ---
## Signif. codes:  0 '***' 0.001 '**' 0.01 '*' 0.05 '.' 0.1 ' ' 1
## 
## Residual standard error: 3.403e-05 on 18148 degrees of freedom
## Multiple R-squared:  0.1245,	Adjusted R-squared:  0.1244 
## F-statistic:  2580 on 1 and 18148 DF,  p-value: < 2.2e-16
## 
## [1] "----- Medium-----"
## 
## Call:
## lm(formula = Duration ~ Size, data = d[d$group == bp[bp$Limit == 
##     lim, ]$Name, ])
## 
## Residuals:
##        Min         1Q     Median         3Q        Max 
## -8.753e-05 -4.022e-05 -1.294e-05  3.682e-05  1.467e-04 
## 
## Coefficients:
##              Estimate Std. Error t value Pr(>|t|)    
## (Intercept) 1.154e-04  8.137e-07  141.79   <2e-16 ***
## Size        2.093e-09  3.793e-11   55.18   <2e-16 ***
## ---
## Signif. codes:  0 '***' 0.001 '**' 0.01 '*' 0.05 '.' 0.1 ' ' 1
## 
## Residual standard error: 5.118e-05 on 8398 degrees of freedom
## Multiple R-squared:  0.2661,	Adjusted R-squared:  0.266 
## F-statistic:  3044 on 1 and 8398 DF,  p-value: < 2.2e-16
## 
## [1] "----- Asynchronous-----"
## 
## Call:
## lm(formula = Duration ~ Size, data = d[d$group == bp[bp$Limit == 
##     lim, ]$Name, ])
## 
## Residuals:
##        Min         1Q     Median         3Q        Max 
## -9.900e-05 -3.870e-05 -1.776e-06  3.596e-05  1.383e-04 
## 
## Coefficients:
##              Estimate Std. Error t value Pr(>|t|)    
## (Intercept) 1.896e-04  5.827e-06   32.53   <2e-16 ***
## Size        1.289e-09  6.288e-11   20.51   <2e-16 ***
## ---
## Signif. codes:  0 '***' 0.001 '**' 0.01 '*' 0.05 '.' 0.1 ' ' 1
## 
## Residual standard error: 4.686e-05 on 1648 degrees of freedom
## Multiple R-squared:  0.2033,	Adjusted R-squared:  0.2028 
## F-statistic: 420.6 on 1 and 1648 DF,  p-value: < 2.2e-16
## 
## [1] "----- Detached-----"
## 
## Call:
## lm(formula = Duration ~ Size, data = d[d$group == bp[bp$Limit == 
##     lim, ]$Name, ])
## 
## Residuals:
##        Min         1Q     Median         3Q        Max 
## -5.368e-04 -6.036e-05  1.770e-06  6.874e-05  5.020e-04 
## 
## Coefficients:
##              Estimate Std. Error t value Pr(>|t|)    
## (Intercept) 1.771e-04  6.043e-06    29.3   <2e-16 ***
## Size        2.925e-09  2.556e-11   114.4   <2e-16 ***
## ---
## Signif. codes:  0 '***' 0.001 '**' 0.01 '*' 0.05 '.' 0.1 ' ' 1
## 
## Residual standard error: 0.0001087 on 2898 degrees of freedom
## Multiple R-squared:  0.8187,	Adjusted R-squared:  0.8187 
## F-statistic: 1.309e+04 on 1 and 2898 DF,  p-value: < 2.2e-16
## 
## [1] "----- Large-----"
## 
## Call:
## lm(formula = Duration ~ Size, data = d[d$group == bp[bp$Limit == 
##     lim, ]$Name, ])
## 
## Residuals:
##        Min         1Q     Median         3Q        Max 
## -8.881e-04 -5.142e-05  2.331e-05  1.025e-04  4.801e-04 
## 
## Coefficients:
##              Estimate Std. Error t value Pr(>|t|)    
## (Intercept) 6.981e-05  1.933e-05   3.612 0.000311 ***
## Size        3.262e-09  2.726e-11 119.659  < 2e-16 ***
## ---
## Signif. codes:  0 '***' 0.001 '**' 0.01 '*' 0.05 '.' 0.1 ' ' 1
## 
## Residual standard error: 0.0001847 on 1998 degrees of freedom
## Multiple R-squared:  0.8775,	Adjusted R-squared:  0.8775 
## F-statistic: 1.432e+04 on 1 and 1998 DF,  p-value: < 2.2e-16
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
## [1] 4.08585e-08
```
MPI_Iprobe timing
----------------
We made 1000 runs of pingpong with pollling on MPI\_Iprobe. Compute the Duration of such a call, and check whether its time is related to the size of the message

![plot of chunk unnamed-chunk-17](figure/unnamed-chunk-17-1.png)
Time for one MPI_Iprobe call

```
## [1] 2.569687e-07
```

MPI_Test timing
---------------

![plot of chunk unnamed-chunk-19](figure/unnamed-chunk-19-1.png)
Time for one MPI_Test call

```
## [1] 2.458362e-07
```


Result of calibration.
---------------

The following snippet of XML has to be included at the beginning of your platformfile. Please report to the SimGrid mailing list any bug with the calibration or the generated platform file.


```
## <config id="General">
##  <prop id="smpi/os" value="0:1.72337677392447e-06:2.80556540986036e-10;1420:0:0;65536:0:0;131072:0:0;450000:0:0"/>
##  <prop id="smpi/ois" value="0:1.79150437795041e-06:2.60957841144294e-10;1420:0:0;65536:0:0;131072:0:0;450000:0:0"/>
##  <prop id="smpi/or" value="0:3.27843243070814e-06:3.41675771019744e-09;1420:0:0;65536:0:0;131072:0:0;450000:0:0"/>
##  <prop id="smpi/bw_factor" value="0:0.057020586149179;1420:0.955647413834744;65536:1.55105118737739;131072:0.683839496893133;450000:0.613154277278705"/>
##  <prop id="smpi/lat_factor" value="0:0.421545218360744;1420:1.15371118224093;65536:1.89572829187563;131072:1.77078433749545;450000:0.698058157888401"/>
##  <prop id="smpi/async_small_thres" value="17408"/>
##  <prop id="smpi/send_is_detached_thres" value="17408"/>
##  <prop id="smpi/wtime" value="4.08585e-08"/>
##  <prop id="smpi/iprobe" value="2.56968726731199e-07"/>
##  <prop id="smpi/test" value="2.45836195508587e-07"/>
## </config>
```

```
## [1] "Results written in testplatform.xml"
```

```
## [1] "testplatform_output.xml"
```

