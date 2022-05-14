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

  <xsl:key name="frequency" match="/plist/array[1]/dict[1]/key[.='CPUPStates'][1]/following-sibling::*[1][self::array]/dict/key[.='Frequency'][1]/following-sibling::*[1][self::integer]" use="ancestor::*[1]/key[.='PState'][1]/following-sibling::*[1][self::integer]" />

  <!-- $ ioreg -ad1 -rn X86PlatformPlugin -->
  <xsl:template match="/">
    <xsl:text>[NOTE] The actual clock rate may be higher.&#10;</xsl:text>

    <xsl:variable name="PStates" select="plist/array[1]/dict[1]/key[.='CPUPStates'][1]/following-sibling::*[1][self::array]" />
    <xsl:variable name="PLimits" select="plist/array[1]/dict[1]/key[.='IOPPFDiagDict'][1]/following-sibling::*[1][self::dict]" />

    <xsl:variable name="PLimits.Version" select="$PLimits/key[.='PLimitVersion'][1]/following-sibling::*[1][self::integer]" />
    <!-- i/o -->
    <xsl:value-of select="concat('PLimits.Version = ', $PLimits.Version, '&#10;')" />

    <xsl:variable name="PLimits.CPU" select="$PLimits/key[.='CPUPLimitDict'][1]/following-sibling::*[1][self::dict]" />
    <xsl:if test="$PLimits.CPU">
      <xsl:variable name="PLimits.CPU.now" select="$PLimits.CPU/key[.='currentLimit'][1]/following-sibling::*[1][self::integer]" />
      <xsl:variable name="PLimits.CPU.min" select="$PLimits.CPU/key[.='pMin'][1]/following-sibling::*[1][self::integer]" />
      <xsl:variable name="PLimits.CPU.max" select="$PLimits.CPU/key[.='pMax'][1]/following-sibling::*[1][self::integer]" />
      <!-- i/o -->
      <xsl:value-of select="concat('PLimits.CPU.now = P', $PLimits.CPU.now, ' (', key('frequency', $PLimits.CPU.now), 'MHz)&#10;')" />
      <xsl:value-of select="concat('PLimits.CPU.min = P', $PLimits.CPU.min, ' (', key('frequency', $PLimits.CPU.min), 'MHz)&#10;')" />
      <xsl:value-of select="concat('PLimits.CPU.max = P', $PLimits.CPU.max, ' (', key('frequency', $PLimits.CPU.max), 'MHz)&#10;')" />
    </xsl:if>

    <xsl:variable name="PLimits.iGPU" select="$PLimits/key[.='IGPUPLimitDict'][1]/following-sibling::*[1][self::dict]" />
    <xsl:if test="$PLimits.iGPU">
      <xsl:variable name="PLimits.iGPU.now" select="$PLimits.iGPU/key[.='currentLimit'][1]/following-sibling::*[1][self::integer]" />
      <xsl:variable name="PLimits.iGPU.min" select="$PLimits.iGPU/key[.='pMin'][1]/following-sibling::*[1][self::integer]" />
      <xsl:variable name="PLimits.iGPU.max" select="$PLimits.iGPU/key[.='pMax'][1]/following-sibling::*[1][self::integer]" />
      <!-- i/o -->
      <xsl:value-of select="concat('PLimits.iGPU.now = P', $PLimits.iGPU.now, '&#10;')" />
      <xsl:value-of select="concat('PLimits.iGPU.min = P', $PLimits.iGPU.min, '&#10;')" />
      <xsl:value-of select="concat('PLimits.iGPU.max = P', $PLimits.iGPU.max, '&#10;')" />
    </xsl:if>

    <xsl:variable name="PLimits.iGPU.SingleSlice" select="$PLimits/key[.='IGPUSingleSlicePLimitDict'][1]/following-sibling::*[1][self::dict]" />
    <xsl:if test="$PLimits.iGPU.SingleSlice">
      <xsl:variable name="PLimits.iGPU.SingleSlice.min" select="$PLimits.iGPU.SingleSlice/key[.='pMin'][1]/following-sibling::*[1][self::integer]" />
      <xsl:variable name="PLimits.iGPU.SingleSlice.max" select="$PLimits.iGPU.SingleSlice/key[.='pMax'][1]/following-sibling::*[1][self::integer]" />
      <!-- i/o -->
      <xsl:value-of select="concat('PLimits.iGPU.SingleSlice.min = P', $PLimits.iGPU.SingleSlice.min, '&#10;')" />
      <xsl:value-of select="concat('PLimits.iGPU.SingleSlice.max = P', $PLimits.iGPU.SingleSlice.max, '&#10;')" />
    </xsl:if>

    <xsl:variable name="PLimits.eGPU" select="$PLimits/key[.='EGPUPLimitDict'][1]/following-sibling::*[1][self::dict]" />
    <xsl:if test="$PLimits.eGPU">
      <xsl:variable name="PLimits.eGPU.now" select="$PLimits.eGPU/key[.='currentLimit'][1]/following-sibling::*[1][self::integer]" />
      <xsl:variable name="PLimits.eGPU.min" select="$PLimits.eGPU/key[.='pMin'][1]/following-sibling::*[1][self::integer]" />
      <xsl:variable name="PLimits.eGPU.max" select="$PLimits.eGPU/key[.='pMax'][1]/following-sibling::*[1][self::integer]" />
      <!-- i/o -->
      <xsl:value-of select="concat('PLimits.eGPU.now = P', $PLimits.eGPU.now, '&#10;')" />
      <xsl:value-of select="concat('PLimits.eGPU.min = P', $PLimits.eGPU.min, '&#10;')" />
      <xsl:value-of select="concat('PLimits.eGPU.max = P', $PLimits.eGPU.max, '&#10;')" />
    </xsl:if>

    <!-- ForcedIdleTable records the graduation of maximum micro/nano-seconds ? -->
    <xsl:variable name="PLimits.Idle" select="$PLimits/key[.='IdlePLimitDict'][1]/following-sibling::*[1][self::dict]" />
    <xsl:if test="$PLimits.Idle">
      <xsl:variable name="PLimits.Idle.now" select="$PLimits.Idle/key[.='currentLimit'][1]/following-sibling::*[1][self::integer]" />
      <xsl:variable name="PLimits.Idle.min" select="$PLimits.Idle/key[.='pMin'][1]/following-sibling::*[1][self::integer]" />
      <xsl:variable name="PLimits.Idle.max" select="$PLimits.Idle/key[.='pMax'][1]/following-sibling::*[1][self::integer]" />
      <!-- i/o -->
      <xsl:value-of select="concat('PLimits.Idle.now = P', $PLimits.Idle.now, '&#10;')" />
      <xsl:value-of select="concat('PLimits.Idle.min = P', $PLimits.Idle.min, '&#10;')" />
      <xsl:value-of select="concat('PLimits.Idle.max = P', $PLimits.Idle.max, '&#10;')" />
    </xsl:if>

    <xsl:text>[NOTE] The normal performance state is P0.&#10;</xsl:text>
  </xsl:template>

</xsl:stylesheet>
