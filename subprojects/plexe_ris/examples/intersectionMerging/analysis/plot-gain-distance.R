library('tikzDevice')
library(RColorBrewer)

source('plot-utils.R')

palette(brewer.pal(4, "RdYlBu"))
a <- palette()
a[3] <- 'black'
palette(a)

#graphs params
plot.width <- 308.43/72
plot.height <- 184.546607/72/1.25
plot.margin <- c(3.1, 3.3, 0.1, 0.1)
leg.inset <- c(0, 0, 0, 0)

plot.gain.distance <- function(outputFile, d) {

    done <- myps(outputFile = outputFile, width=plot.width, height=plot.height)
    par(mar=plot.margin, xpd=F)
    plot.new()

    plot.window(xlim=c(400, -10), ylim=c(0, 62), yaxs="i", xaxs="i")

    lines(d$antennaDistance, d$antennaGain, lwd=2, lty=1, col=3)

    axis(1, lwd=0, lwd.ticks=1, las=1)
    axis(2, lwd=0, lwd.ticks=1, las=1)
    title(ylab='gain [linear]', line=2.3)
    title(xlab='distance to intersection [m]', line=2)
    box()
    done()
}
plot.angles.distance <- function(outputFile, d) {

    done <- myps(outputFile = outputFile, width=plot.width, height=plot.height)
    par(mar=plot.margin, xpd=F)
    plot.new()

    plot.window(xlim=c(400, -10), ylim=c(-135, 145), yaxs="i", xaxs="i")

    lines(d$antennaDistance, d$phiR * 180/pi,   lwd=2, col=1, lty=1)
    lines(d$antennaDistance, d$thetaR * 180/pi, lwd=2, col=2, lty=2)
    lines(d$antennaDistance, d$phiI * 180/pi,   lwd=2, col=3, lty=3)
    lines(d$antennaDistance, d$thetaI * 180/pi, lwd=2, col=4, lty=4)

    axis(1, lwd=0, lwd.ticks=1, las=1)
    axis(2, lwd=0, lwd.ticks=1, las=1, at=seq(-135, 135, by=45))
    title(ylab='angle [$^{\\circ}$]', line=2.3)
    title(xlab='distance to intersection [m]', line=2)
    box()

    legend(
        "topleft",
        c("reflection azimuth $\\phi_R$", "reflection elevation $\\theta_R$", "incidence azimuth $\\phi_I$", "incidence elevation $\\theta_I$"),
        bty="n",
        col=1:4,
        pch=NA,
        lty=1:4,
        ncol=2,
        lwd=2,
        box.lwd=0,
        pt.bg=NA,
        pt.cex=NA,
        cex=.6,
        seg.len=3,
        inset=c(0, 0)
    )



    done()
}

plot.pathloss.distance <- function(outputFile, d, txpower) {

    done <- myps(outputFile = outputFile, width=plot.width, height=plot.height)
    par(mar=plot.margin, xpd=F)
    plot.new()


    plot.window(xlim=c(400, -10), ylim=c(80, 145), yaxs="i", xaxs="i")

    lines(d$antennaDistance, txpower - d$antennaPower, lwd=2, lty=1, col=3)
    lines(d$antennaDistance, txpower - d$antennaPower + 10*log10(d$antennaGain), lwd=2, lty=2, col=2)

    axis(1, lwd=0, lwd.ticks=1, las=1)
    axis(2, lwd=0, lwd.ticks=1, las=1)
    title(ylab='path loss [dB]', line=2.3)
    title(xlab='distance to intersection [m]', line=2)
    box()
    legend(
        "topleft",
        c("w/ RIS", "w/o RIS"),
        bty="n",
        col=c(3,2),
        pch=NA,
        lty=1:2,
        ncol=2,
        lwd=2,
        box.lwd=0,
        pt.bg=NA,
        pt.cex=NA,
        cex=.6,
        seg.len=3,
        inset=c(0, 0)
    )
    done()
}

load('../results/GainDistance.Rdata')
d <- subset(allData, antennaId == 0)
d <- d[order(d$time),]
d <- subset(d, antennaDistance < 1e6)
# d <- subset(d, antennaGain != 0)
plot.gain.distance("distance-vs-gain", d)
plot.angles.distance("distance-vs-angles", d)
plot.pathloss.distance("distance-vs-pathloss", d, 30)
