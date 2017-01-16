load <- function() {
   df <- read.csv("taurus_PingPong.csv", header=F,sep=",")
 max((df$V2/(1024*1024))/df$V4)
}
load()

