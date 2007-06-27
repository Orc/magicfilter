dnl
dnl Build the output-postscript filter; if this is a postscript file,
dnl just dump it to the printer, otherwise if a DEVICE is specified
dnl and either DPI or PS_OPTIONS are set, run ghostscript, else fall
dnl over dead.
dnl
define(PSopts,-q -dSAFER -dNOPAUSE ifdef(`papersize',`-sPAPERSIZE='papersize ,)ifdef(`DEVICE',`-sDEVICE='DEVICE ,)ifdef(`DPI', `-r'DPI ,)defn(PS_OPTIONS) -sOutputFile=${FIFO} - -c quit)dnl
define(PSfilter,ifelse(POSTSCRIPT,true,
			    postscript,
			    ifdef(`PATH_GS',
				ifdef(`DEVICE', filter PATH_GS PSopts,
				    ifdef(`PS_OPTIONS', filter PATH_GS PSopts,
					    reject)),reject)))dnl
define(PCLfilter, ifelse(PCL,true,cat,reject))dnl
define(PJLfilter, ifelse(PJL,true,cat,ifelse(PCL,true,cat,reject)))dnl
define(PSfromText, ifelse(defn(`PSfilter'), reject,
			    reject,
			    ifdef(`PATH_PSTEXT',
				pipe/postscript/ PATH_PSTEXT,
				    ifdef(`PATH_NENSCRIPT',
					pipe/postscript/ PATH_NENSCRIPT -qp -,
					reject))))dnl
#
`# magicfilter for 'ifdef(`Vendor',`the 'Vendor` ',)Printer
#

ifelse(defn(`PSfilter'),reject,,
#
# Portable imagemaps.
#
/p[gbp]m/	ifdef(`PATH_PNMTOPS',pipe/postscript/ PATH_PNMTOPS -quiet, reject)

#
# picture formats.
#
/gif/              ifdef(`PATH_GIFTOPPM',pipe/p[gbp]m/ PATH_GIFTOPPM,reject)
/jpeg/             ifdef(`PATH_DJPEG',pipe/p[gbp]m/ PATH_DJPEG -pnm,reject)
/PNG image/        ifdef(`PATH_PNGTOPNM',pipe/p[gbp]m/ PATH_PNGTOPNM,reject)
/TIFF image/       ifdef(`PATH_TIFFTOPNM',pipe/p[gbp]m/ PATH_TIFFTOPNM,reject)
/PC bitmap data/   ifdef(`PATH_BMPTOPPM',pipe/p[gbp]m/ PATH_BMPTOPPM,reject)
/Sun raster image/ ifdef(`PATH_RASTTOPNM',pipe/p[gbp]m/ PATH_RASTTOPNM,reject)
/SGI image data/   ifdef(`PATH_SGITOPNM',pipe/p[gbp]m/ PATH_SGITOPNM,reject)

#
# Miscellaneous formats.
#
/fig/		ifdef(`PATH_FIGTODEV',pipe/postscript/ PATH_FIGTODEV -Lps -P -l dummy,reject)
/dvi/		ifdef(`PATH_DVIPS', fpipe/postscript/ PATH_DVIPS ifdef(`XDPI',-X XDPI -Y YDPI, -D DPI) -R -q -f,reject)
/fax.*normal/	ifdef(`PATH_G3TOPBM',pipe/p[gbp]m/ PATH_G3TOPBM -stretch,reject)
/fax/		ifdef(`PATH_G3TOPBM',pipe/p[gbp]m/ PATH_G3TOPBM,reject)
)dnl

/pdf/		ifdef(`PATH_ACROREAD',pipe/postscript/ PATH_ACROREAD -toPostScript,ifdef(`PATH_GS',filter PATH_GS ifdef(`DEVICE',,`-sDEVICE=psgray') PSopts,reject))

#
# compressed formats.
#
/compress/	ifdef(`PATH_ZCAT', pipe PATH_ZCAT, ifdef(`PATH_GZIP', pipe PATH_GZIP -dc, reject))
/gzip/		ifdef(`PATH_GZIP', pipe PATH_GZIP -dc, reject)
/packed/	ifdef(`PATH_GZIP', pipe PATH_GZIP -dc, reject)
/frozen/	ifdef(`PATH_GZIP', pipe PATH_GZIP -dc, reject)
/lzh/		ifdef(`PATH_GZIP', pipe PATH_GZIP -dc, reject)
/bzip/		ifdef(`PATH_BZIP', pipe PATH_BZIP -dc, reject)

dnl
dnl after everything else, put out the methods for printing postscript,
dnl pcl, and pjl.
dnl
#
# printer languages.
#
/postscript/	PSfilter
/pcl/		PCLfilter
/pjl/		PJLfilter

dnl
dnl default print policy:  if HANDLE_TEXT is defined, use that to
dnl print the file, otherwise if TEXT is not false, just cat it, otherwise
dnl if a PS filter is defined, use pstext, else else reject it
dnl
#
# text, finally
#
/text/		ifdef(`HANDLE_TEXT', defn(`HANDLE_TEXT'),
		    ifelse(defn(`TEXT'),false,
			PSfromText,
			text defn(`TEXT_ARGS')))
# EOF
