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



MPI_Send timing
---------------

Timings for this experiment are taken from a ping-pong experiment, used to determine os.

We determine the piecewiese regression based on information taken from the regression file pointed in the XML configuration file


```
##        Limit         Name LimitInf
## 1       1420        Small        0
## 2      32768       Medium     1420
## 3      65536 Asynchronous    32768
## 4     270000     Detached    65536
## 5 2147483647        Large   270000
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
## -3.678e-07 -8.078e-08 -2.810e-09  7.388e-08  1.483e-06 
## 
## Coefficients:
##              Estimate Std. Error t value Pr(>|t|)    
## (Intercept) 5.761e-06  1.207e-09 4772.67   <2e-16 ***
## Size        2.174e-10  3.035e-12   71.64   <2e-16 ***
## ---
## Signif. codes:  0 '***' 0.001 '**' 0.01 '*' 0.05 '.' 0.1 ' ' 1
## 
## Residual standard error: 1.219e-07 on 14536 degrees of freedom
## Multiple R-squared:  0.2609,	Adjusted R-squared:  0.2609 
## F-statistic:  5132 on 1 and 14536 DF,  p-value: < 2.2e-16
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
## -3.626e-07 -9.910e-08 -3.330e-08  4.810e-08  1.978e-05 
## 
## Coefficients:
##              Estimate Std. Error t value Pr(>|t|)    
## (Intercept) 5.694e-06  2.561e-09 2222.90   <2e-16 ***
## Size        1.868e-10  6.437e-12   29.02   <2e-16 ***
## ---
## Signif. codes:  0 '***' 0.001 '**' 0.01 '*' 0.05 '.' 0.1 ' ' 1
## 
## Residual standard error: 2.588e-07 on 14540 degrees of freedom
## Multiple R-squared:  0.05477,	Adjusted R-squared:  0.0547 
## F-statistic: 842.4 on 1 and 14540 DF,  p-value: < 2.2e-16
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
## -4.373e-07 -5.489e-08 -2.150e-09  5.087e-08  7.370e-07 
## 
## Coefficients:
##              Estimate Std. Error t value Pr(>|t|)    
## (Intercept) 4.525e-06  8.692e-10  5206.4   <2e-16 ***
## Size        2.352e-10  2.184e-12   107.7   <2e-16 ***
## ---
## Signif. codes:  0 '***' 0.001 '**' 0.01 '*' 0.05 '.' 0.1 ' ' 1
## 
## Residual standard error: 8.783e-08 on 14544 degrees of freedom
## Multiple R-squared:  0.4436,	Adjusted R-squared:  0.4436 
## F-statistic: 1.16e+04 on 1 and 14544 DF,  p-value: < 2.2e-16
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
## -1.101e-06 -4.817e-07 -2.885e-07 -6.790e-08  3.061e-05 
## 
## Coefficients:
##              Estimate Std. Error t value Pr(>|t|)    
## (Intercept) 1.380e-05  1.017e-08 1356.88   <2e-16 ***
## Size        6.867e-10  2.556e-11   26.86   <2e-16 ***
## ---
## Signif. codes:  0 '***' 0.001 '**' 0.01 '*' 0.05 '.' 0.1 ' ' 1
## 
## Residual standard error: 1.148e-06 on 18148 degrees of freedom
## Multiple R-squared:  0.03824,	Adjusted R-squared:  0.03819 
## F-statistic: 721.6 on 1 and 18148 DF,  p-value: < 2.2e-16
## 
## [1] "----- Medium-----"
## 
## Call:
## lm(formula = Duration ~ Size, data = d[d$group == bp[bp$Limit == 
##     lim, ]$Name, ])
## 
## Residuals:
##        Min         1Q     Median         3Q        Max 
## -4.292e-06 -9.630e-07 -3.525e-07  1.528e-07  2.207e-05 
## 
## Coefficients:
##              Estimate Std. Error t value Pr(>|t|)    
## (Intercept) 1.397e-05  3.932e-08   355.3   <2e-16 ***
## Size        7.058e-10  2.864e-12   246.4   <2e-16 ***
## ---
## Signif. codes:  0 '***' 0.001 '**' 0.01 '*' 0.05 '.' 0.1 ' ' 1
## 
## Residual standard error: 2.063e-06 on 7248 degrees of freedom
## Multiple R-squared:  0.8934,	Adjusted R-squared:  0.8934 
## F-statistic: 6.074e+04 on 1 and 7248 DF,  p-value: < 2.2e-16
## 
## [1] "----- Asynchronous-----"
## 
## Call:
## lm(formula = Duration ~ Size, data = d[d$group == bp[bp$Limit == 
##     lim, ]$Name, ])
## 
## Residuals:
##        Min         1Q     Median         3Q        Max 
## -4.788e-06 -1.247e-06 -6.556e-07  1.880e-07  2.073e-05 
## 
## Coefficients:
##              Estimate Std. Error t value Pr(>|t|)    
## (Intercept) 2.043e-05  4.457e-07   45.83   <2e-16 ***
## Size        7.538e-10  9.561e-12   78.84   <2e-16 ***
## ---
## Signif. codes:  0 '***' 0.001 '**' 0.01 '*' 0.05 '.' 0.1 ' ' 1
## 
## Residual standard error: 2.766e-06 on 1148 degrees of freedom
## Multiple R-squared:  0.8441,	Adjusted R-squared:  0.844 
## F-statistic:  6216 on 1 and 1148 DF,  p-value: < 2.2e-16
## 
## [1] "----- Detached-----"
## 
## Call:
## lm(formula = Duration ~ Size, data = d[d$group == bp[bp$Limit == 
##     lim, ]$Name, ])
## 
## Residuals:
##        Min         1Q     Median         3Q        Max 
## -7.241e-06 -1.827e-06 -3.945e-07  1.109e-06  2.638e-05 
## 
## Coefficients:
##              Estimate Std. Error t value Pr(>|t|)    
## (Intercept) 6.280e-05  1.457e-07   431.0   <2e-16 ***
## Size        4.068e-10  9.248e-13   439.9   <2e-16 ***
## ---
## Signif. codes:  0 '***' 0.001 '**' 0.01 '*' 0.05 '.' 0.1 ' ' 1
## 
## Residual standard error: 3.325e-06 on 3898 degrees of freedom
## Multiple R-squared:  0.9803,	Adjusted R-squared:  0.9802 
## F-statistic: 1.935e+05 on 1 and 3898 DF,  p-value: < 2.2e-16
## 
## [1] "----- Large-----"
## 
## Call:
## lm(formula = Duration ~ Size, data = d[d$group == bp[bp$Limit == 
##     lim, ]$Name, ])
## 
## Residuals:
##        Min         1Q     Median         3Q        Max 
## -8.772e-06 -2.763e-06 -9.990e-07  1.115e-06  5.162e-05 
## 
## Coefficients:
##              Estimate Std. Error t value Pr(>|t|)    
## (Intercept) 6.412e-05  3.000e-07   213.8   <2e-16 ***
## Size        4.063e-10  4.689e-13   866.6   <2e-16 ***
## ---
## Signif. codes:  0 '***' 0.001 '**' 0.01 '*' 0.05 '.' 0.1 ' ' 1
## 
## Residual standard error: 4.861e-06 on 2648 degrees of freedom
## Multiple R-squared:  0.9965,	Adjusted R-squared:  0.9965 
## F-statistic: 7.51e+05 on 1 and 2648 DF,  p-value: < 2.2e-16
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

  
  ```
  ## [1] 2.257284e-10 3.529039e-10 3.768967e-10 2.034196e-10 2.031695e-10
  ```
MPI_Wtime timing
---------------

We made a run with 10 millions calls to MPI\_Wtime and we want to know the time of one call


Time for one MPI_Wtime call

```
## [1] 4.749348e-08
```
MPI_Iprobe timing
----------------
We made 1000 runs of pingpong with pollling on MPI\_Iprobe. Compute the Duration of such a call, and check whether its time is related to the size of the message

![plot of chunk unnamed-chunk-17](figure/unnamed-chunk-17-1.png)
Time for one MPI_Iprobe call

```
## [1] 8.486947e-07
```

MPI_Test timing
---------------

![plot of chunk unnamed-chunk-19](figure/unnamed-chunk-19-1.png)
Time for one MPI_Test call

```
## [1] 7.854703e-07
```


Result of calibration.
---------------

The following snippet of XML has to be included at the beginning of your platformfile. Please report to the SimGrid mailing list any bug with the calibration or the generated platform file.


```
## <config id="General">
##  <prop id="smpi/os" value="0:5.76056454253197e-06:2.17436977784238e-10;1420:0:0;32768:0:0;65536:0:0;270000:0:0"/>
##  <prop id="smpi/ois" value="0:5.69394882068122e-06:1.86831851358575e-10;1420:6.0526761228944e-06:1.29229526276907e-10;32768:6.63727561499294e-06:1.58841990243663e-10;65536:6.08546668295576e-06:3.45048519321393e-12;270000:7.00313634805895e-06:1.40299146883574e-13"/>
##  <prop id="smpi/or" value="0:4.52519352462666e-06:2.35225923712713e-10;1420:0:0;32768:0:0;65536:0:0;270000:0:0"/>
##  <prop id="smpi/bw-factor" value="0:0.632871947922852;1420:0.404804603515132;32768:0.379035226587968;65536:0.702278038393304;270000:0.703142529187253"/>
##  <prop id="smpi/lat-factor" value="0:1;1420:1;32768:1.02143038745115;65536:3.13978860642611;270000:3.20623358192956"/>
##  <prop id="smpi/async-small-thres" value="17408"/>
##  <prop id="smpi/send-is-detached-thres" value="17408"/>
##  <prop id="smpi/wtime" value="4.749348e-08"/>
##  <prop id="smpi/iprobe" value="8.48694690265487e-07"/>
##  <prop id="smpi/test" value="7.8547027027027e-07"/>
## </config>
```

```
## [1] "Results written in taurus.xml"
```

```
## [1] "taurus_output.xml"
```

