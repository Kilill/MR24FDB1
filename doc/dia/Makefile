svgfiles =  MR24FDB1_fall.svg MR24FDB1_get.svg MR24FDB1_page1.svg MR24FDB1_reports.svg MR24FDB1_responce.svg MR24FDB1_set.svg Protocol.svg

all: $(svgfiles)

$(svgfiles): %.svg: %.dia
	dia -e $@ -t svg $<
	
MR24FDB1_fall.svg: MR24FDB1_fall.dia
MR24FDB1_get.svg: MR24FDB1_get.dia
MR24FDB1_page1.svg: MR24FDB1_page1.dia
MR24FDB1_reports.svg: MR24FDB1_reports.dia
MR24FDB1_responce.svg: MR24FDB1_responce.dia
MR24FDB1_set.svg: MR24FDB1_set.dia
Protocol.svg: Protocol.dia
