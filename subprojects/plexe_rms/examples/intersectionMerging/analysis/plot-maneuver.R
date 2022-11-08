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

plot.virtual.distance <- function(outputFile, d) {

    done <- myps(outputFile = outputFile, width=plot.width, height=plot.height)
    par(mar=plot.margin, xpd=F)
    plot.new()

    plot.window(xlim=xlim, ylim=c(-15, 38), yaxs="i", xaxs="i")

    # choose only the leaders of platoons B and C
    ids <- c(2, 4)
    d <- subset(d, nodeId %in% ids)
    d <- d[order(d$time),]

    for (i in 1:length(ids)) {
        toplot <- subset(d, nodeId == ids[i])
        lines(toplot$time, toplot$virtualDistance, lwd=2, lty=i+1, col=i)
    }

    axis(1, lwd=0, lwd.ticks=1, las=1)
    axis(2, lwd=0, lwd.ticks=1, las=1)
    title(ylab='virtual distance [m]', line=2.3)
    title(xlab='time [s]', line=2)
    box()
    legend(
        "topleft",
        c("leader B", "leader C"),
        bty="n",
        col=1:2,
        pch=NA,
        lty=2:3,
        ncol=ncols,
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

plot.distance <- function(outputFile, d) {

    done <- myps(outputFile = outputFile, width=plot.width, height=plot.height)
    par(mar=plot.margin, xpd=F)
    plot.new()

    plot.window(xlim=xlim, ylim=c(0, 72), yaxs="i", xaxs="i")

    # choose only the leaders of platoons B and C
    ids <- c(2, 4)
    d <- subset(d, nodeId %in% ids)
    d <- d[order(d$time),]
    d$distance[d$distance < 0] <- NA

    for (i in 1:length(ids)) {
        toplot <- subset(d, nodeId == ids[i])
        lines(toplot$time, toplot$distance, lwd=2, lty=i+1, col=i)
    }

    axis(1, lwd=0, lwd.ticks=1, las=1)
    axis(2, lwd=0, lwd.ticks=1, las=1)
    title(ylab='distance [m]', line=2.3)
    title(xlab='time [s]', line=2)
    box()
    legend(
        "topleft",
        c("leader B", "leader C"),
        bty="n",
        col=1:2,
        pch=NA,
        lty=2:3,
        ncol=ncols,
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

plot.speed <- function(outputFile, d) {

    done <- myps(outputFile = outputFile, width=plot.width, height=plot.height)
    par(mar=plot.margin, xpd=F)
    plot.new()

    plot.window(xlim=xlim, ylim=c(0, 65), yaxs="i", xaxs="i")

    # choose only the leaders of platoons A B and C
    ids <- c(0, 2, 4)
    cols <- c(3, 1, 2)
    d <- subset(d, nodeId %in% ids)
    d <- d[order(d$time),]

    for (i in 1:length(ids)) {
        toplot <- subset(d, nodeId == ids[i])
        lines(toplot$time, toplot$speed * 3.6, lwd=2, lty=i, col=cols[i])
    }

    axis(1, lwd=0, lwd.ticks=1, las=1)
    axis(2, lwd=0, lwd.ticks=1, las=1)
    title(ylab='speed [km/h]', line=2.3)
    title(xlab='time [s]', line=2)
    box()
    legend(
        "topleft",
        c("leader A", "leader B", "leader C"),
        bty="n",
        col=cols,
        pch=NA,
        lty=1:3,
        ncol=ncols,
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

plot.acceleration <- function(outputFile, d) {

    done <- myps(outputFile = outputFile, width=plot.width, height=plot.height)
    par(mar=plot.margin, xpd=F)
    plot.new()

    plot.window(xlim=xlim, ylim=c(-4, 5), yaxs="i", xaxs="i")

    # choose only the leaders of platoons A B and C
    ids <- c(0, 2, 4)
    cols <- c(3, 1, 2)
    d <- subset(d, nodeId %in% ids)
    d <- d[order(d$time),]

    for (i in 1:length(ids)) {
        toplot <- subset(d, nodeId == ids[i])
        lines(toplot$time, toplot$acceleration, lwd=2, lty=i, col=cols[i])
    }

    axis(1, lwd=0, lwd.ticks=1, las=1)
    axis(2, lwd=0, lwd.ticks=1, las=1)
    title(ylab='acceleration [m/s$^2$]', line=2.3)
    title(xlab='time [s]', line=2)
    box()
    legend(
        "topleft",
        c("leader A", "leader B", "leader C"),
        bty="n",
        col=cols,
        pch=NA,
        lty=1:3,
        ncol=ncols,
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

xlim = c(0, 90)
ncols = 1

load('../results/IntersectionMerge.Rdata')
allData$virtualDistance[allData$virtualDistance < -1e5] <- NA
# shadowing
for (s in c(0, 1)) {
    # rms
    for (r in c(0, 1)) {
        if (s == 0 && r == 1) next
        d <- subset(allData, time > 3 & shadowing==s & useRms==r)
        plot.virtual.distance(paste("virtual-distance", s, r, sep="_"), d)
        plot.speed(paste("speed", s, r, sep="_"), d)
        plot.acceleration(paste("acceleration", s, r, sep="_"), d)
        plot.distance(paste("distance", s, r, sep="_"), d)
    }
}
