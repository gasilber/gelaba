<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:xsd="http://www.w3.org/2001/XMLSchema" version="1.0">
    <xsl:output method="text" encoding="utf-8"/>
    <xsl:param name="sourcepath" select="'source/loms'"/>
    <xsl:param name="tocprefix" select="'loms'"/>
    <xsl:param name="part" select="'rules'"/>
    <xsl:variable name="lowercase" select="'abcdefghijklmnopqrstuvwxyz'" />
    <xsl:variable name="uppercase" select="'ABCDEFGHIJKLMNOPQRSTUVWXYZ'" />
    <xsl:template match="/">
        <xsl:choose>
            <xsl:when test="$part = 'targets'">
                <xsl:apply-templates select="/xsd:schema/xsd:element" mode="target"/>
                <xsl:apply-templates select="/xsd:schema/xsd:complexType" mode="target"/>
                <xsl:apply-templates select="/xsd:schema/xsd:simpleType" mode="target"/>
            </xsl:when>
            <xsl:when test="$part = 'tocs'">
                <xsl:apply-templates select="/xsd:schema/xsd:element" mode="tocs"/>
                <xsl:apply-templates select="/xsd:schema/xsd:complexType" mode="tocs"/>
                <xsl:apply-templates select="/xsd:schema/xsd:simpleType" mode="tocs"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:apply-templates select="/xsd:schema/xsd:element"/>
                <xsl:apply-templates select="/xsd:schema/xsd:complexType"/>
                <xsl:apply-templates select="/xsd:schema/xsd:simpleType"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template match="/xsd:schema/xsd:element[@name]|xsd:simpleType[@name]|xsd:complexType[@name]">
        <xsl:variable name="nname">
            <xsl:call-template name="type-name">
                <xsl:with-param name="tname" select="@name"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:value-of select="$sourcepath"/>
        <xsl:text>/</xsl:text>
        <xsl:value-of select="translate($nname, $uppercase, $lowercase)"/>
        <xsl:text>.rst: $(TORST) $(LOMS)</xsl:text>
        <xsl:text>&#10;</xsl:text>
        <xsl:text>&#9;$(XSLTPROC) -o $@ --stringparam typename </xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:text> $(TORST) $(LOMS)</xsl:text>
        <xsl:text>&#10;</xsl:text>
    </xsl:template>
    <xsl:template match="/xsd:schema/xsd:element[@name]|xsd:simpleType[@name]|xsd:complexType[@name]"
            mode="target">
        <xsl:variable name="nname">
            <xsl:call-template name="type-name">
                <xsl:with-param name="tname" select="@name"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:value-of select="$sourcepath"/>
        <xsl:text>/</xsl:text>
        <xsl:value-of select="translate($nname, $uppercase, $lowercase)"/>
        <xsl:text>.rst \&#10;</xsl:text>
    </xsl:template>
    <xsl:template match="/xsd:schema/xsd:element[@name]|xsd:simpleType[@name]|xsd:complexType[@name]"
            mode="tocs">
        <xsl:variable name="nname">
            <xsl:call-template name="type-name">
                <xsl:with-param name="tname" select="@name"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:text>   </xsl:text>
        <xsl:value-of select="$tocprefix"/>
        <xsl:text>/</xsl:text>
        <xsl:value-of select="translate($nname, $uppercase, $lowercase)"/>
        <xsl:text>&#10;</xsl:text>
    </xsl:template>
    <xsl:template match="text()" />
    <xsl:template name="type-name">
        <xsl:param name="tname"/>
        <xsl:variable name="tpos" select="string-length($tname) - 3"/>
        <xsl:variable name="lastfour" select="substring($tname, $tpos, 4)"/>
        <xsl:choose>
            <xsl:when test="$lastfour = 'Type'">
                <xsl:value-of select="substring($tname, 0, $tpos)"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="$tname"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
</xsl:stylesheet>