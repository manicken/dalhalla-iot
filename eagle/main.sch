<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE eagle SYSTEM "eagle.dtd">
<eagle version="7.7.0">
<drawing>
<settings>
<setting alwaysvectorfont="no"/>
<setting verticaltext="up"/>
</settings>
<grid distance="0.1" unitdist="inch" unit="inch" style="lines" multiple="1" display="no" altdistance="0.01" altunitdist="inch" altunit="inch"/>
<layers>
<layer number="1" name="Top" color="4" fill="1" visible="no" active="no"/>
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
<library name="ESP-32s-adapter-breakout">
<packages>
<package name="ESP-32S-ADAPTER-PCB">
<pad name="IO23" x="12.7" y="5.08" drill="0.8"/>
<pad name="RXD" x="15.24" y="5.08" drill="0.8"/>
<pad name="IO32" x="-12.7" y="5.08" drill="0.8"/>
<pad name="IO33" x="-15.24" y="5.08" drill="0.8"/>
<pad name="IO14" x="-15.24" y="0" drill="0.8"/>
<pad name="IO26" x="-15.24" y="2.54" drill="0.8"/>
<pad name="IO35" x="-15.24" y="7.62" drill="0.8"/>
<pad name="SVN" x="-15.24" y="10.16" drill="0.8"/>
<pad name="IO13" x="-15.24" y="-2.54" drill="0.8"/>
<pad name="SD3" x="-15.24" y="-5.08" drill="0.8"/>
<pad name="CLK" x="-15.24" y="-7.62" drill="0.8"/>
<pad name="SDI" x="-15.24" y="-10.16" drill="0.8"/>
<pad name="GND2" x="15.24" y="2.54" drill="0.8"/>
<pad name="IO21" x="15.24" y="0" drill="0.8"/>
<pad name="IO18" x="15.24" y="-2.54" drill="0.8"/>
<pad name="TXD" x="15.24" y="7.62" drill="0.8"/>
<pad name="3V3" x="15.24" y="10.16" drill="0.8"/>
<pad name="IO17" x="15.24" y="-5.08" drill="0.8"/>
<pad name="IO4" x="15.24" y="-7.62" drill="0.8"/>
<pad name="IO2" x="15.24" y="-10.16" drill="0.8"/>
<pad name="IO15" x="12.7" y="-10.16" drill="0.8"/>
<pad name="IO0" x="12.7" y="-7.62" drill="0.8"/>
<pad name="IO16" x="12.7" y="-5.08" drill="0.8"/>
<pad name="IO5" x="12.7" y="-2.54" drill="0.8"/>
<pad name="IO19" x="12.7" y="0" drill="0.8"/>
<pad name="IO22" x="12.7" y="2.54" drill="0.8"/>
<pad name="EN" x="12.7" y="7.62" drill="0.8"/>
<pad name="GND" x="12.7" y="10.16" drill="0.8"/>
<pad name="SVP" x="-12.7" y="10.16" drill="0.8"/>
<pad name="IO34" x="-12.7" y="7.62" drill="0.8"/>
<pad name="IO25" x="-12.7" y="2.54" drill="0.8"/>
<pad name="IO27" x="-12.7" y="0" drill="0.8"/>
<pad name="IO12" x="-12.7" y="-2.54" drill="0.8"/>
<pad name="SD2" x="-12.7" y="-5.08" drill="0.8"/>
<pad name="CMD" x="-12.7" y="-7.62" drill="0.8"/>
<pad name="SDO" x="-12.7" y="-10.16" drill="0.8"/>
<wire x1="-17.78" y1="19.05" x2="-17.78" y2="-19.05" width="0.127" layer="21"/>
<wire x1="-17.78" y1="-19.05" x2="17.78" y2="-19.05" width="0.127" layer="21"/>
<wire x1="17.78" y1="-19.05" x2="17.78" y2="19.05" width="0.127" layer="21"/>
<wire x1="17.78" y1="19.05" x2="-17.78" y2="19.05" width="0.127" layer="21"/>
<wire x1="10.16" y1="12.7" x2="10.16" y2="17.78" width="0.127" layer="21"/>
<wire x1="10.16" y1="17.78" x2="7.62" y2="17.78" width="0.127" layer="21"/>
<wire x1="7.62" y1="17.78" x2="7.62" y2="12.7" width="0.127" layer="21"/>
<wire x1="7.62" y1="17.78" x2="5.08" y2="17.78" width="0.127" layer="21"/>
<wire x1="5.08" y1="17.78" x2="5.08" y2="15.24" width="0.127" layer="21"/>
<wire x1="5.08" y1="15.24" x2="2.54" y2="15.24" width="0.127" layer="21"/>
<wire x1="2.54" y1="15.24" x2="2.54" y2="17.78" width="0.127" layer="21"/>
<wire x1="2.54" y1="17.78" x2="0" y2="17.78" width="0.127" layer="21"/>
<wire x1="0" y1="17.78" x2="0" y2="15.24" width="0.127" layer="21"/>
<wire x1="0" y1="15.24" x2="-2.54" y2="15.24" width="0.127" layer="21"/>
<wire x1="-2.54" y1="15.24" x2="-2.54" y2="17.78" width="0.127" layer="21"/>
<wire x1="-2.54" y1="17.78" x2="-5.08" y2="17.78" width="0.127" layer="21"/>
<wire x1="-5.08" y1="17.78" x2="-5.08" y2="15.24" width="0.127" layer="21"/>
<wire x1="-5.08" y1="15.24" x2="-7.62" y2="15.24" width="0.127" layer="21"/>
<wire x1="-7.62" y1="15.24" x2="-7.62" y2="17.78" width="0.127" layer="21"/>
<wire x1="-7.62" y1="17.78" x2="-10.16" y2="17.78" width="0.127" layer="21"/>
<wire x1="-10.16" y1="17.78" x2="-10.16" y2="15.24" width="0.127" layer="21"/>
<pad name="GND3" x="12.7" y="10.16" drill="0.8"/>
</package>
</packages>
<symbols>
<symbol name="ESP-32S-ADAPTER-BREAKOUT">
<pin name="GND@1" x="-7.62" y="20.32" length="middle"/>
<pin name="3V3" x="-7.62" y="17.78" length="middle"/>
<pin name="EN" x="-7.62" y="15.24" length="middle"/>
<pin name="SVP" x="-7.62" y="12.7" length="middle"/>
<pin name="SNP" x="-7.62" y="10.16" length="middle"/>
<pin name="IO34" x="-7.62" y="7.62" length="middle"/>
<pin name="IO35" x="-7.62" y="5.08" length="middle"/>
<pin name="IO32" x="-7.62" y="2.54" length="middle"/>
<pin name="IO33" x="-7.62" y="0" length="middle"/>
<pin name="IO25" x="-7.62" y="-2.54" length="middle"/>
<pin name="IO26" x="-7.62" y="-5.08" length="middle"/>
<pin name="IO27" x="-7.62" y="-7.62" length="middle"/>
<pin name="IO14" x="-7.62" y="-10.16" length="middle"/>
<pin name="IO12" x="-7.62" y="-12.7" length="middle"/>
<pin name="GND@15" x="-7.62" y="-15.24" length="middle"/>
<pin name="IO13" x="-7.62" y="-17.78" length="middle"/>
<pin name="IO15" x="27.94" y="-17.78" length="middle" rot="R180"/>
<pin name="IO2" x="27.94" y="-15.24" length="middle" rot="R180"/>
<pin name="IO0" x="27.94" y="-12.7" length="middle" rot="R180"/>
<pin name="IO4" x="27.94" y="-10.16" length="middle" rot="R180"/>
<pin name="NC@27" x="27.94" y="-7.62" length="middle" rot="R180"/>
<pin name="NC@28" x="27.94" y="-5.08" length="middle" rot="R180"/>
<pin name="IO5" x="27.94" y="-2.54" length="middle" rot="R180"/>
<pin name="IO18" x="27.94" y="0" length="middle" rot="R180"/>
<pin name="IO19" x="27.94" y="2.54" length="middle" rot="R180"/>
<pin name="IO21" x="27.94" y="7.62" length="middle" rot="R180"/>
<pin name="RXD0" x="27.94" y="10.16" length="middle" rot="R180"/>
<pin name="TXD0" x="27.94" y="12.7" length="middle" rot="R180"/>
<pin name="IO22" x="27.94" y="15.24" length="middle" rot="R180"/>
<pin name="IO23" x="27.94" y="17.78" length="middle" rot="R180"/>
<pin name="GND@38" x="27.94" y="20.32" length="middle" rot="R180"/>
<wire x1="-2.54" y1="22.86" x2="-2.54" y2="-27.94" width="0.254" layer="94"/>
<wire x1="-2.54" y1="-27.94" x2="22.86" y2="-27.94" width="0.254" layer="94"/>
<wire x1="22.86" y1="-27.94" x2="22.86" y2="22.86" width="0.254" layer="94"/>
<wire x1="22.86" y1="22.86" x2="-2.54" y2="22.86" width="0.254" layer="94"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="ESP-32S-ADAPTER-BREAKOUT" prefix="ESP-32S-ADAPTER-PCB">
<gates>
<gate name="G$1" symbol="ESP-32S-ADAPTER-BREAKOUT" x="-10.16" y="0"/>
</gates>
<devices>
<device name="" package="ESP-32S-ADAPTER-PCB">
<connects>
<connect gate="G$1" pin="3V3" pad="3V3"/>
<connect gate="G$1" pin="EN" pad="EN"/>
<connect gate="G$1" pin="GND@1" pad="GND"/>
<connect gate="G$1" pin="GND@15" pad="GND2"/>
<connect gate="G$1" pin="GND@38" pad="GND3"/>
<connect gate="G$1" pin="IO0" pad="IO0"/>
<connect gate="G$1" pin="IO12" pad="IO12"/>
<connect gate="G$1" pin="IO13" pad="IO13"/>
<connect gate="G$1" pin="IO14" pad="IO14"/>
<connect gate="G$1" pin="IO15" pad="IO15"/>
<connect gate="G$1" pin="IO18" pad="IO18"/>
<connect gate="G$1" pin="IO19" pad="IO19"/>
<connect gate="G$1" pin="IO2" pad="IO2"/>
<connect gate="G$1" pin="IO21" pad="IO21"/>
<connect gate="G$1" pin="IO22" pad="IO22"/>
<connect gate="G$1" pin="IO23" pad="IO23"/>
<connect gate="G$1" pin="IO25" pad="IO25"/>
<connect gate="G$1" pin="IO26" pad="IO26"/>
<connect gate="G$1" pin="IO27" pad="IO27"/>
<connect gate="G$1" pin="IO32" pad="IO32"/>
<connect gate="G$1" pin="IO33" pad="IO33"/>
<connect gate="G$1" pin="IO34" pad="IO34"/>
<connect gate="G$1" pin="IO35" pad="IO35"/>
<connect gate="G$1" pin="IO4" pad="IO4"/>
<connect gate="G$1" pin="IO5" pad="IO5"/>
<connect gate="G$1" pin="NC@27" pad="IO16"/>
<connect gate="G$1" pin="NC@28" pad="IO17"/>
<connect gate="G$1" pin="RXD0" pad="RXD"/>
<connect gate="G$1" pin="SNP" pad="SVN"/>
<connect gate="G$1" pin="SVP" pad="SVP"/>
<connect gate="G$1" pin="TXD0" pad="TXD"/>
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
<part name="ESP-32S-ADAPTER-PCB1" library="ESP-32s-adapter-breakout" deviceset="ESP-32S-ADAPTER-BREAKOUT" device=""/>
</parts>
<sheets>
<sheet>
<plain>
</plain>
<instances>
<instance part="ESP-32S-ADAPTER-PCB1" gate="G$1" x="53.34" y="55.88"/>
</instances>
<busses>
</busses>
<nets>
</nets>
</sheet>
</sheets>
</schematic>
</drawing>
</eagle>
