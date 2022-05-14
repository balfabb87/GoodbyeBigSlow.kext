<?xml version="1.0" encoding="UTF-8"?>
<!-- Copyright (c) 2022 by J.W https://github.com/jakwings/GoodbyeBigSlow.kext
   -
   -   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
   -
   -  0. You just DO WHAT THE FUCK YOU WANT TO.
   -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                              xmlns:x="http://www.w3.org/1999/xhtml">

  <xsl:output method="text" encoding="UTF-8" indent="no" />

  <xsl:param name="nCPU.max" select="'?'" />

  <!-- $ ioreg -ad1 -rn IOPMrootDomain -->
  <xsl:template match="/">
    <xsl:text>[NOTE] The actual processor speed may be higher.&#10;</xsl:text>

    <xsl:variable name="PowerStatus.CPU" select="plist/array[1]/dict[1]/key[.='Power Status'][1]/following-sibling::*[1][self::dict]/key[.='CPU_Power_Limits'][1]/following-sibling::*[1][self::dict]" />
    <xsl:if test="$PowerStatus.CPU">
      <xsl:variable name="PowerStatus.CPU.Time" select="$PowerStatus.CPU/key[.='CPU_Scheduler_Limit'][1]/following-sibling::*[1][self::integer]" />
      <xsl:variable name="PowerStatus.CPU.Speed" select="$PowerStatus.CPU/key[.='CPU_Speed_Limit'][1]/following-sibling::*[1][self::integer]" />
      <xsl:variable name="PowerStatus.CPU.Available" select="$PowerStatus.CPU/key[.='CPU_Available_CPUs'][1]/following-sibling::*[1][self::integer]" />
      <!-- i/o -->
      <xsl:value-of select="concat('PowerStatus.CPU.Time = ', $PowerStatus.CPU.Time, '%&#10;')" />
      <xsl:value-of select="concat('PowerStatus.CPU.Speed = ', $PowerStatus.CPU.Speed, '%&#10;')" />
      <xsl:value-of select="concat('PowerStatus.CPU.Available = ', $PowerStatus.CPU.Available, '/', $nCPU.max, '&#10;')" />
    </xsl:if>

  </xsl:template>

</xsl:stylesheet>
