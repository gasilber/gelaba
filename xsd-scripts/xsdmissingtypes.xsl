<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:xsd="http://www.w3.org/2001/XMLSchema" version="1.0">
    <xsl:output method="text" encoding="utf-8"/>
    <xsl:template match="//xsd:element[not(@type)]">
        <xsl:value-of select="@name" />
        <xsl:text> </xsl:text>
        <xsl:value-of select="xsd:complexType/xsd:complexContent/xsd:restriction/@base"/>
        <xsl:text>&#10;</xsl:text>
    </xsl:template>
    <xsl:template match="text()" />
</xsl:stylesheet>