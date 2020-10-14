<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE eagle SYSTEM "eagle.dtd">
<eagle version="9.5.1">
<drawing>
<settings>
<setting alwaysvectorfont="no"/>
<setting verticaltext="up"/>
</settings>
<grid distance="0.1" unitdist="inch" unit="inch" style="lines" multiple="1" display="no" altdistance="0.01" altunitdist="inch" altunit="inch"/>
<layers>
<layer number="1" name="Top" color="4" fill="1" visible="no" active="no"/>
<layer number="2" name="Route2" color="1" fill="3" visible="no" active="no"/>
<layer number="3" name="Route3" color="4" fill="3" visible="no" active="no"/>
<layer number="4" name="Route4" color="1" fill="4" visible="no" active="no"/>
<layer number="5" name="Route5" color="4" fill="4" visible="no" active="no"/>
<layer number="6" name="Route6" color="1" fill="8" visible="no" active="no"/>
<layer number="7" name="Route7" color="4" fill="8" visible="no" active="no"/>
<layer number="8" name="Route8" color="1" fill="2" visible="no" active="no"/>
<layer number="9" name="Route9" color="4" fill="2" visible="no" active="no"/>
<layer number="10" name="Route10" color="1" fill="7" visible="no" active="no"/>
<layer number="11" name="Route11" color="4" fill="7" visible="no" active="no"/>
<layer number="12" name="Route12" color="1" fill="5" visible="no" active="no"/>
<layer number="13" name="Route13" color="4" fill="5" visible="no" active="no"/>
<layer number="14" name="Route14" color="1" fill="6" visible="no" active="no"/>
<layer number="15" name="Route15" color="4" fill="6" visible="no" active="no"/>
<layer number="16" name="Bottom" color="1" fill="1" visible="no" active="no"/>
<layer number="17" name="Pads" color="2" fill="1" visible="no" active="no"/>
<layer number="18" name="Vias" color="2" fill="1" visible="no" active="no"/>
<layer number="19" name="Unrouted" color="6" fill="1" visible="no" active="no"/>
<layer number="20" name="Dimension" color="15" fill="1" visible="no" active="no"/>
<layer number="21" name="tPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="22" name="bPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="23" name="tOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="24" name="bOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="25" name="tNames" color="7" fill="1" visible="no" active="no"/>
<layer number="26" name="bNames" color="7" fill="1" visible="no" active="no"/>
<layer number="27" name="tValues" color="7" fill="1" visible="no" active="no"/>
<layer number="28" name="bValues" color="7" fill="1" visible="no" active="no"/>
<layer number="29" name="tStop" color="7" fill="3" visible="no" active="no"/>
<layer number="30" name="bStop" color="7" fill="6" visible="no" active="no"/>
<layer number="31" name="tCream" color="7" fill="4" visible="no" active="no"/>
<layer number="32" name="bCream" color="7" fill="5" visible="no" active="no"/>
<layer number="33" name="tFinish" color="6" fill="3" visible="no" active="no"/>
<layer number="34" name="bFinish" color="6" fill="6" visible="no" active="no"/>
<layer number="35" name="tGlue" color="7" fill="4" visible="no" active="no"/>
<layer number="36" name="bGlue" color="7" fill="5" visible="no" active="no"/>
<layer number="37" name="tTest" color="7" fill="1" visible="no" active="no"/>
<layer number="38" name="bTest" color="7" fill="1" visible="no" active="no"/>
<layer number="39" name="tKeepout" color="4" fill="11" visible="no" active="no"/>
<layer number="40" name="bKeepout" color="1" fill="11" visible="no" active="no"/>
<layer number="41" name="tRestrict" color="4" fill="10" visible="no" active="no"/>
<layer number="42" name="bRestrict" color="1" fill="10" visible="no" active="no"/>
<layer number="43" name="vRestrict" color="2" fill="10" visible="no" active="no"/>
<layer number="44" name="Drills" color="7" fill="1" visible="no" active="no"/>
<layer number="45" name="Holes" color="7" fill="1" visible="no" active="no"/>
<layer number="46" name="Milling" color="3" fill="1" visible="no" active="no"/>
<layer number="47" name="Measures" color="7" fill="1" visible="no" active="no"/>
<layer number="48" name="Document" color="7" fill="1" visible="no" active="no"/>
<layer number="49" name="Reference" color="7" fill="1" visible="no" active="no"/>
<layer number="51" name="tDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="52" name="bDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="88" name="SimResults" color="9" fill="1" visible="yes" active="yes"/>
<layer number="89" name="SimProbes" color="9" fill="1" visible="yes" active="yes"/>
<layer number="90" name="Modules" color="5" fill="1" visible="yes" active="yes"/>
<layer number="91" name="Nets" color="2" fill="1" visible="yes" active="yes"/>
<layer number="92" name="Busses" color="1" fill="1" visible="yes" active="yes"/>
<layer number="93" name="Pins" color="2" fill="1" visible="no" active="yes"/>
<layer number="94" name="Symbols" color="4" fill="1" visible="yes" active="yes"/>
<layer number="95" name="Names" color="7" fill="1" visible="yes" active="yes"/>
<layer number="96" name="Values" color="7" fill="1" visible="yes" active="yes"/>
<layer number="97" name="Info" color="7" fill="1" visible="yes" active="yes"/>
<layer number="98" name="Guide" color="6" fill="1" visible="yes" active="yes"/>
</layers>
<schematic xreflabel="%F%N/%S.%C%R" xrefpart="/%S.%C%R">
<libraries>
<library name="frames" urn="urn:adsk.eagle:library:229">
<description>&lt;b&gt;Frames for Sheet and Layout&lt;/b&gt;</description>
<packages>
</packages>
<symbols>
<symbol name="A3L-LOC" urn="urn:adsk.eagle:symbol:13881/1" library_version="1">
<wire x1="288.29" y1="3.81" x2="342.265" y2="3.81" width="0.1016" layer="94"/>
<wire x1="342.265" y1="3.81" x2="373.38" y2="3.81" width="0.1016" layer="94"/>
<wire x1="373.38" y1="3.81" x2="383.54" y2="3.81" width="0.1016" layer="94"/>
<wire x1="383.54" y1="3.81" x2="383.54" y2="8.89" width="0.1016" layer="94"/>
<wire x1="383.54" y1="8.89" x2="383.54" y2="13.97" width="0.1016" layer="94"/>
<wire x1="383.54" y1="13.97" x2="383.54" y2="19.05" width="0.1016" layer="94"/>
<wire x1="383.54" y1="19.05" x2="383.54" y2="24.13" width="0.1016" layer="94"/>
<wire x1="288.29" y1="3.81" x2="288.29" y2="24.13" width="0.1016" layer="94"/>
<wire x1="288.29" y1="24.13" x2="342.265" y2="24.13" width="0.1016" layer="94"/>
<wire x1="342.265" y1="24.13" x2="383.54" y2="24.13" width="0.1016" layer="94"/>
<wire x1="373.38" y1="3.81" x2="373.38" y2="8.89" width="0.1016" layer="94"/>
<wire x1="373.38" y1="8.89" x2="383.54" y2="8.89" width="0.1016" layer="94"/>
<wire x1="373.38" y1="8.89" x2="342.265" y2="8.89" width="0.1016" layer="94"/>
<wire x1="342.265" y1="8.89" x2="342.265" y2="3.81" width="0.1016" layer="94"/>
<wire x1="342.265" y1="8.89" x2="342.265" y2="13.97" width="0.1016" layer="94"/>
<wire x1="342.265" y1="13.97" x2="383.54" y2="13.97" width="0.1016" layer="94"/>
<wire x1="342.265" y1="13.97" x2="342.265" y2="19.05" width="0.1016" layer="94"/>
<wire x1="342.265" y1="19.05" x2="383.54" y2="19.05" width="0.1016" layer="94"/>
<wire x1="342.265" y1="19.05" x2="342.265" y2="24.13" width="0.1016" layer="94"/>
<text x="344.17" y="15.24" size="2.54" layer="94">&gt;DRAWING_NAME</text>
<text x="344.17" y="10.16" size="2.286" layer="94">&gt;LAST_DATE_TIME</text>
<text x="357.505" y="5.08" size="2.54" layer="94">&gt;SHEET</text>
<text x="343.916" y="4.953" size="2.54" layer="94">Sheet:</text>
<frame x1="0" y1="0" x2="387.35" y2="260.35" columns="8" rows="5" layer="94"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="A3L-LOC" urn="urn:adsk.eagle:component:13942/1" prefix="FRAME" uservalue="yes" library_version="1">
<description>&lt;b&gt;FRAME&lt;/b&gt;&lt;p&gt;
DIN A3, landscape with location and doc. field</description>
<gates>
<gate name="G$1" symbol="A3L-LOC" x="0" y="0"/>
</gates>
<devices>
<device name="">
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="afig-0007">
<packages>
<package name="AFIG-0007">
<pad name="P$1" x="-2.9" y="3.2" drill="1"/>
<pad name="P$2" x="2.9" y="3.2" drill="1"/>
<pad name="P$3" x="2.9" y="-3.2" drill="1" rot="R180"/>
<pad name="P$4" x="-2.9" y="-3.2" drill="1" rot="R180"/>
<wire x1="-3.7" y1="4.6" x2="-3.7" y2="-4.6" width="0.127" layer="51"/>
<wire x1="3.7" y1="4.6" x2="3.7" y2="-4.6" width="0.127" layer="51"/>
<wire x1="-3.7" y1="-4.6" x2="-3" y2="-4.6" width="0.127" layer="51"/>
<wire x1="-3" y1="-4.6" x2="3" y2="-4.6" width="0.127" layer="51"/>
<wire x1="3" y1="-4.6" x2="3.7" y2="-4.6" width="0.127" layer="51"/>
<wire x1="-3.7" y1="4.6" x2="-2.1" y2="4.6" width="0.127" layer="51"/>
<wire x1="-2.1" y1="4.6" x2="2.1" y2="4.6" width="0.127" layer="51"/>
<wire x1="2.1" y1="4.6" x2="3.7" y2="4.6" width="0.127" layer="51"/>
<wire x1="-3.7" y1="4.6" x2="-3.7" y2="7.2" width="0.127" layer="51"/>
<wire x1="-3.7" y1="7.2" x2="-2.1" y2="7.2" width="0.127" layer="51"/>
<wire x1="-2.1" y1="7.2" x2="-2.1" y2="4.6" width="0.127" layer="51"/>
<wire x1="3.7" y1="4.6" x2="3.7" y2="7.2" width="0.127" layer="51"/>
<wire x1="2.1" y1="7.2" x2="3.7" y2="7.2" width="0.127" layer="51"/>
<wire x1="2.1" y1="7.2" x2="2.1" y2="4.6" width="0.127" layer="51"/>
<wire x1="3.7" y1="-4.6" x2="3.7" y2="-7.2" width="0.127" layer="51"/>
<wire x1="3.7" y1="-7.2" x2="2.1" y2="-7.2" width="0.127" layer="51"/>
<wire x1="2.1" y1="-7.2" x2="2.1" y2="-4.6" width="0.127" layer="51"/>
<wire x1="-3.7" y1="-4.6" x2="-3.7" y2="-7.2" width="0.127" layer="51"/>
<wire x1="-2.1" y1="-7.2" x2="-3.7" y2="-7.2" width="0.127" layer="51"/>
<wire x1="-2.1" y1="-7.2" x2="-2.1" y2="-4.6" width="0.127" layer="51"/>
<wire x1="-3" y1="-4.6" x2="-3" y2="-9.6" width="0.127" layer="51"/>
<wire x1="-3" y1="-9.6" x2="3" y2="-9.6" width="0.127" layer="51"/>
<wire x1="3" y1="-9.6" x2="3" y2="-4.6" width="0.127" layer="51"/>
</package>
<package name="0878580002">
<smd name="P$1" x="-1" y="0" dx="1" dy="2.5" layer="1"/>
<smd name="P$2" x="1" y="0" dx="1" dy="2.5" layer="1"/>
<hole x="0" y="2.94" drill="1.2"/>
<wire x1="-2" y1="3.94" x2="-2" y2="1.94" width="0.127" layer="51"/>
<wire x1="2" y1="3.94" x2="2" y2="1.94" width="0.127" layer="51"/>
<wire x1="-2" y1="3.94" x2="2" y2="3.94" width="0.127" layer="51"/>
<wire x1="-2" y1="1.94" x2="2" y2="1.94" width="0.127" layer="51"/>
<wire x1="-1.3" y1="1.9" x2="-1.3" y2="-0.9" width="0.1" layer="51"/>
<wire x1="-0.7" y1="1.9" x2="-0.7" y2="-0.9" width="0.1" layer="51"/>
<wire x1="-1.3" y1="-0.9" x2="-0.7" y2="-0.9" width="0.1" layer="51"/>
<wire x1="0.7" y1="1.9" x2="0.7" y2="-0.9" width="0.1" layer="51"/>
<wire x1="1.3" y1="1.9" x2="1.3" y2="-0.9" width="0.1" layer="51"/>
<wire x1="0.7" y1="-0.9" x2="1.3" y2="-0.9" width="0.1" layer="51"/>
<wire x1="-1.3" y1="7" x2="-1.3" y2="4" width="0.1" layer="51"/>
<wire x1="-0.7" y1="7" x2="-0.7" y2="4" width="0.1" layer="51"/>
<wire x1="0.7" y1="7" x2="0.7" y2="4" width="0.1" layer="51"/>
<wire x1="1.3" y1="7" x2="1.3" y2="4" width="0.1" layer="51"/>
<wire x1="-1.3" y1="7" x2="-1.2" y2="7.2" width="0.1" layer="51"/>
<wire x1="-0.7" y1="7" x2="-0.8" y2="7.2" width="0.1" layer="51"/>
<wire x1="-1.2" y1="7.2" x2="-0.8" y2="7.2" width="0.1" layer="51"/>
<wire x1="0.7" y1="7" x2="0.8" y2="7.2" width="0.1" layer="51"/>
<wire x1="1.3" y1="7" x2="1.2" y2="7.2" width="0.1" layer="51"/>
<wire x1="0.8" y1="7.2" x2="1.2" y2="7.2" width="0.1" layer="51"/>
</package>
</packages>
<symbols>
<symbol name="AFIG-0007">
<wire x1="0" y1="10.16" x2="0" y2="8.89" width="0.254" layer="94"/>
<wire x1="0" y1="8.89" x2="0" y2="7.62" width="0.254" layer="94" curve="-180"/>
<wire x1="0" y1="7.62" x2="0" y2="6.35" width="0.254" layer="94" curve="-180"/>
<wire x1="0" y1="6.35" x2="0" y2="5.08" width="0.254" layer="94" curve="-180"/>
<wire x1="0" y1="5.08" x2="0" y2="3.81" width="0.254" layer="94" curve="-180"/>
<wire x1="0" y1="3.81" x2="0" y2="2.54" width="0.254" layer="94"/>
<wire x1="0" y1="2.54" x2="2.54" y2="2.54" width="0.254" layer="94"/>
<wire x1="0" y1="-10.16" x2="0" y2="-8.89" width="0.254" layer="94"/>
<wire x1="0" y1="-8.89" x2="0" y2="-7.62" width="0.254" layer="94" curve="180"/>
<wire x1="0" y1="-7.62" x2="0" y2="-6.35" width="0.254" layer="94" curve="180"/>
<wire x1="0" y1="-6.35" x2="0" y2="-5.08" width="0.254" layer="94" curve="180"/>
<wire x1="0" y1="-5.08" x2="0" y2="-3.81" width="0.254" layer="94" curve="180"/>
<wire x1="0" y1="-3.81" x2="0" y2="-2.54" width="0.254" layer="94"/>
<wire x1="0" y1="-2.54" x2="2.54" y2="-2.54" width="0.254" layer="94"/>
<pin name="P$1" x="0" y="10.16" length="point"/>
<pin name="P$2" x="2.54" y="2.54" length="point"/>
<pin name="P$3" x="0" y="-10.16" length="point"/>
<pin name="P$4" x="2.54" y="-2.54" length="point"/>
<circle x="-1.27" y="3.81" radius="0.635" width="0.254" layer="94"/>
<circle x="-1.27" y="-8.89" radius="0.635" width="0.254" layer="94"/>
</symbol>
<symbol name="0878580002">
<pin name="P$1" x="10.16" y="0" length="middle" rot="R180"/>
<pin name="P$2" x="10.16" y="-5.08" length="middle" rot="R180"/>
<wire x1="5.08" y1="2.54" x2="5.08" y2="-7.62" width="0.254" layer="94"/>
<wire x1="5.08" y1="-7.62" x2="-5.08" y2="-7.62" width="0.254" layer="94"/>
<wire x1="-5.08" y1="-7.62" x2="-5.08" y2="2.54" width="0.254" layer="94"/>
<wire x1="-5.08" y1="2.54" x2="5.08" y2="2.54" width="0.254" layer="94"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="AFIG-0007">
<gates>
<gate name="G$1" symbol="AFIG-0007" x="0" y="0"/>
</gates>
<devices>
<device name="" package="AFIG-0007">
<connects>
<connect gate="G$1" pin="P$1" pad="P$1"/>
<connect gate="G$1" pin="P$2" pad="P$2"/>
<connect gate="G$1" pin="P$3" pad="P$3"/>
<connect gate="G$1" pin="P$4" pad="P$4"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="0878580002">
<gates>
<gate name="G$1" symbol="0878580002" x="0" y="0"/>
</gates>
<devices>
<device name="" package="0878580002">
<connects>
<connect gate="G$1" pin="P$1" pad="P$1"/>
<connect gate="G$1" pin="P$2" pad="P$2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
</libraries>
<attributes>
</attributes>
<variantdefs>
</variantdefs>
<classes>
<class number="0" name="default" width="0" drill="0">
</class>
</classes>
<parts>
<part name="FRAME1" library="frames" library_urn="urn:adsk.eagle:library:229" deviceset="A3L-LOC" device=""/>
<part name="U$1" library="afig-0007" deviceset="AFIG-0007" device=""/>
<part name="U$2" library="afig-0007" deviceset="0878580002" device=""/>
</parts>
<sheets>
<sheet>
<plain>
</plain>
<instances>
<instance part="FRAME1" gate="G$1" x="0" y="-261.62" smashed="yes">
<attribute name="DRAWING_NAME" x="344.17" y="-246.38" size="2.54" layer="94"/>
<attribute name="LAST_DATE_TIME" x="344.17" y="-251.46" size="2.286" layer="94"/>
<attribute name="SHEET" x="357.505" y="-256.54" size="2.54" layer="94"/>
</instance>
<instance part="U$1" gate="G$1" x="137.16" y="-106.68" smashed="yes"/>
<instance part="U$2" gate="G$1" x="116.84" y="-104.14" smashed="yes"/>
</instances>
<busses>
</busses>
<nets>
<net name="N$1" class="0">
<segment>
<pinref part="U$1" gate="G$1" pin="P$2"/>
<pinref part="U$1" gate="G$1" pin="P$4"/>
<wire x1="139.7" y1="-104.14" x2="139.7" y2="-109.22" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$3" class="0">
<segment>
<pinref part="U$1" gate="G$1" pin="P$3"/>
<wire x1="137.16" y1="-116.84" x2="137.16" y2="-119.38" width="0.1524" layer="91"/>
<wire x1="137.16" y1="-119.38" x2="129.54" y2="-119.38" width="0.1524" layer="91"/>
<wire x1="129.54" y1="-119.38" x2="129.54" y2="-109.22" width="0.1524" layer="91"/>
<pinref part="U$2" gate="G$1" pin="P$2"/>
<wire x1="129.54" y1="-109.22" x2="127" y2="-109.22" width="0.1524" layer="91"/>
</segment>
</net>
<net name="N$4" class="0">
<segment>
<pinref part="U$1" gate="G$1" pin="P$1"/>
<wire x1="137.16" y1="-96.52" x2="137.16" y2="-93.98" width="0.1524" layer="91"/>
<pinref part="U$2" gate="G$1" pin="P$1"/>
<wire x1="127" y1="-104.14" x2="129.54" y2="-104.14" width="0.1524" layer="91"/>
<wire x1="129.54" y1="-104.14" x2="129.54" y2="-93.98" width="0.1524" layer="91"/>
<wire x1="129.54" y1="-93.98" x2="137.16" y2="-93.98" width="0.1524" layer="91"/>
</segment>
</net>
</nets>
</sheet>
</sheets>
</schematic>
</drawing>
<compatibility>
<note version="8.2" severity="warning">
Since Version 8.2, EAGLE supports online libraries. The ids
of those online libraries will not be understood (or retained)
with this version.
</note>
<note version="8.3" severity="warning">
Since Version 8.3, EAGLE supports URNs for individual library
assets (packages, symbols, and devices). The URNs of those assets
will not be understood (or retained) with this version.
</note>
</compatibility>
</eagle>
