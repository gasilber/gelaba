<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:xsd="http://www.w3.org/2001/XMLSchema" version="1.0">
    <xsl:output method="text" encoding="utf-8"/>
    <xsl:param name="mode" select="'element'"/>
    <xsl:template match="/">
        <xsl:choose>
            <xsl:when test="$mode = 'element'">
                <xsl:apply-templates mode="element"/>
            </xsl:when>
            <xsl:when test="$mode = 'simple'">
                <xsl:apply-templates mode="simple"/>
            </xsl:when>
            <xsl:when test="$mode = 'complex'">
                <xsl:apply-templates mode="complex"/>
            </xsl:when>
        </xsl:choose>
    </xsl:template>
    <xsl:template match="/xsd:schema/xsd:element[@name]" mode="element">
        <xsl:value-of select="@name" />
        <xsl:text>&#10;</xsl:text>
    </xsl:template>
    <xsl:template match="xsd:simpleType[@name]" mode="simple">
        <xsl:value-of select="@name" />
        <xsl:text>&#10;</xsl:text>
    </xsl:template>
    <xsl:template match="xsd:complexType[@name]" mode="complex">
        <xsl:value-of select="@name" />
        <xsl:text>&#10;</xsl:text>
    </xsl:template>
    <xsl:template match="text()" />
    <xsl:template match="text()" mode="element"/>
    <xsl:template match="text()" mode="simple"/>
    <xsl:template match="text()" mode="complex"/>

</xsl:stylesheet>