<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:xsd="http://www.w3.org/2001/XMLSchema"
                xmlns:xrst="http://silbercafe.net/xrst/1.0"
                version="1.0">
    <xsl:output method="text" encoding="utf-8"/>
    <xsl:param name="typename" select="''"/>
    <xsl:variable name="lowercase" select="'abcdefghijklmnopqrstuvwxyz'" />
    <xsl:variable name="uppercase" select="'ABCDEFGHIJKLMNOPQRSTUVWXYZ'" />
    <xsl:template match="/xsd:schema/xsd:element[@name]">
        <xsl:if test="@name = $typename">
            <xsl:apply-templates select="." mode="rstdoc"/>
        </xsl:if>
    </xsl:template>
    <xsl:template match="xsd:simpleType[@name]">
        <xsl:if test="@name = $typename">
            <xsl:apply-templates select="." mode="rstdoc"/>
        </xsl:if>
    </xsl:template>
    <xsl:template match="xsd:complexType[@name]">
        <xsl:if test="@name = $typename">
            <xsl:apply-templates select="." mode="rstdoc"/>
        </xsl:if>
    </xsl:template>
    <xsl:template match="xsd:simpleType|xsd:complexType|xsd:element" mode="rstdoc">
        <xsl:variable name="title">
            <xsl:call-template name="type-name">
                <xsl:with-param name="tname" select="@name"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:text>.. _</xsl:text>
        <xsl:value-of select="translate($title, $uppercase, $lowercase)"/>
        <xsl:text>-type:</xsl:text>
        <xsl:text>&#10;</xsl:text>
        <xsl:text>&#10;</xsl:text>
        <xsl:value-of select="$title"/>
        <xsl:text>&#10;</xsl:text>
        <xsl:call-template name="underline">
            <xsl:with-param name="count" select="string-length($title)"/>
        </xsl:call-template>
        <xsl:text>&#10;</xsl:text>
        <xsl:text>&#10;</xsl:text>
        <xsl:apply-templates select="./xsd:annotation/xsd:documentation" mode="rstdoc"/>
        <xsl:text>&#10;</xsl:text>
        <xsl:text>&#10;</xsl:text>
        <xsl:if test="@type">
            <xsl:text>Content has type </xsl:text> 
            <xsl:call-template name="type-reference">
                <xsl:with-param name="tname" select="@type" />
            </xsl:call-template>
            <xsl:text>.&#10;&#10;</xsl:text>
        </xsl:if>
        <xsl:apply-templates select="./xsd:sequence" mode="rstdoc-xml"/>
        <xsl:apply-templates select="./xsd:choice" mode="rstdoc-xml"/>
        <xsl:apply-templates select="./xsd:sequence" mode="rstdoc-table"/>
        <xsl:apply-templates select="./xsd:choice" mode="rstdoc-table"/>
        <xsl:apply-templates select="./xsd:simpleContent/xsd:extension" mode="rstdoc-table"/>
        <xsl:apply-templates select="./xsd:simpleContent/xsd:restriction" mode="rstdoc-table"/>
        <xsl:apply-templates select="./xsd:complexContent/xsd:restriction" mode="rstdoc-table"/>
        <xsl:apply-templates select="./xsd:complexContent/xsd:extension" mode="rstdoc-table"/>
        <xsl:apply-templates select="./xsd:restriction" mode="rstdoc-table"/>
        <xsl:apply-templates select="./xsd:union" />

        <xsl:if test="count(xsd:attribute) &gt; 0">
            <!--
            <xsl:text>&#10;</xsl:text>        
            <xsl:text>.. list-table::&#10;</xsl:text>
            <xsl:text>    :widths: 25 25 50&#10;</xsl:text>
            <xsl:text>    :header-rows: 1&#10;&#10;</xsl:text>
            <xsl:text>    * - Attribute&#10;</xsl:text>
            <xsl:text>      - Type&#10;</xsl:text>
            <xsl:text>      - Description&#10;</xsl:text>
            -->
            <xsl:text>Attributes&#10;-----------&#10;&#10;</xsl:text>
            <xsl:apply-templates select="xsd:attribute" mode="rstdocattr-table"/>
            <xsl:text>&#10;</xsl:text>
        </xsl:if>

        <!-- custom "embedded" type: complexType/complexContent/restriction -->
        <xsl:if test="count(xsd:complexType/complexContent/restriction) &gt; 0">
            <xsl:text></xsl:text>
        </xsl:if>

    </xsl:template>
    <xsl:template match="xsd:documentation" mode="rstdoc">
        <xsl:apply-templates select="*|text()" mode="rstdoc"/>
    </xsl:template>
    <xsl:template match="xrst:p" mode="rstdoc">
        <xsl:apply-templates select="text()" mode="rstdoc"/>
        <xsl:text>&#10;</xsl:text>
    </xsl:template>
    <xsl:template match="xrst:ul" mode="rstdoc">
        <xsl:text>&#10;</xsl:text>
        <xsl:apply-templates select="*|text()" mode="rstdoc"/>
        <xsl:text>&#10;</xsl:text>
    </xsl:template>
    <xsl:template match="xrst:li" mode="rstdoc">
        <xsl:text>- </xsl:text>
        <xsl:apply-templates select="text()" mode="rstdoc"/>
        <xsl:text>&#10;</xsl:text>
    </xsl:template>
    <xsl:template match="xrst:ref" mode="rstdoc">
        <xsl:text> </xsl:text>
        <xsl:call-template name="type-reference">
            <xsl:with-param name="tname" select="normalize-space(text())"/>
        </xsl:call-template>
        <xsl:text> </xsl:text>
    </xsl:template>
    <xsl:template match="text()" mode="rstdoc">
        <xsl:value-of select="normalize-space(.)"/>
    </xsl:template>
    <xsl:template match="xsd:sequence[count(xsd:element) &gt; 0]" mode="rstdoc-xml">
        <xsl:text>.. code-block:: xml&#10;&#10;</xsl:text>
        <xsl:text>  &lt;!-- SEQUENCE --&gt;&#10;</xsl:text>
        <!--<xsd:if test="count(./xsd:group) &gt; 0">
            <xsd:for-each select="./xsd:group">
                <xsd:text>Group </xsd:text>
                <xsd:value-of select="@ref"/>
            </xsd:for-each>
        </xsd:if>-->
        <xsl:apply-templates select="xsd:group" mode="rstdocelement-xml"/>
        <xsl:apply-templates select="xsd:element" mode="rstdocelement-xml"/>
        <xsl:text>&#10;</xsl:text>
    </xsl:template>
    <xsl:template match="xsd:sequence[xsd:choice]" mode="rstdoc-xml">
        <xsl:apply-templates select="xsd:choice" mode="rstdoc-xml"/>
    </xsl:template>
    <xsl:template match="xsd:choice" mode="rstdoc-xml">
        <xsl:text>.. code-block:: xml&#10;&#10;</xsl:text>
        <xsl:text>  &lt;!-- CHOICE </xsl:text>
        <xsl:apply-templates select="." mode="min"/>
        <xsl:text>,</xsl:text>
        <xsl:apply-templates select="." mode="max"/>
        <xsl:text> --&gt;&#10;</xsl:text>
        <xsl:apply-templates select="xsd:element" mode="rstdocchoiceelement-xml"/>
        <xsl:text>&#10;</xsl:text>
    </xsl:template>
    <xsl:template match="xsd:element" mode="rstdocelement-xml">
        <xsl:text>  &lt;</xsl:text><xsl:value-of select="@name"/><xsl:text>&gt;</xsl:text>
        <xsl:text>...</xsl:text>
        <!--<xsl:call-template name="type-name">
            <xsl:with-param name="tname" select="@type"/>
        </xsl:call-template>
        <xsl:text>...</xsl:text>-->
        <xsl:text>&lt;/</xsl:text><xsl:value-of select="@name"/><xsl:text>&gt;</xsl:text>
        <xsl:text> &lt;!-- </xsl:text>
        <xsl:apply-templates select="." mode="min"/>
        <xsl:text>,</xsl:text>
        <xsl:apply-templates select="." mode="max"/>
        <xsl:text> --&gt;</xsl:text>
        <xsl:text>&#10;</xsl:text>
    </xsl:template>
    <xsl:template match="xsd:group" mode="rstdocelement-xml">
        <!--<xsl:text>  Group </xsl:text><xsl:value-of select="@ref"/><xsl:text>&#10;</xsl:text>-->
        <xsl:variable name="groupname" select="@ref"/>
        <xsl:variable name="groupnode" select="//xsd:group[@name=$groupname]" />
        <!--<xsl:value-of select="$groupnode"/>-->
        <!--<xsl:value-of select="$groupnode/@name"/>-->
        <xsl:apply-templates select="$groupnode" mode="rstdocelements-xml"/>
    </xsl:template>
    <xsl:template match="xsd:group" mode="rstdocelements-xml">
        <xsl:text>  &lt;!-- begin group </xsl:text><xsl:value-of select="@name"/><xsl:text> --&gt;&#10;</xsl:text>
        <xsl:apply-templates select="xsd:sequence/xsd:element" mode="rstdocelement-xml"/>
        <xsl:text>  &lt;!-- end group </xsl:text><xsl:value-of select="@name"/><xsl:text> --&gt;&#10;</xsl:text>
    </xsl:template>
    <xsl:template match="xsd:element" mode="rstdocchoiceelement-xml">
        <xsl:text>  &lt;</xsl:text><xsl:value-of select="@name"/><xsl:text>&gt;</xsl:text>
        <xsl:text>...</xsl:text>
        <!--<xsl:call-template name="type-name">
            <xsl:with-param name="tname" select="@type"/>
        </xsl:call-template>
        <xsl:text>...</xsl:text>-->
        <xsl:text>&lt;/</xsl:text><xsl:value-of select="@name"/><xsl:text>&gt;</xsl:text>
        <xsl:text>&#10;</xsl:text>
    </xsl:template>
    <xsl:template match="xsd:sequence" mode="rstdoc-table">
        <!--
        <xsl:text>.. list-table::&#10;</xsl:text>
        <xsl:text>    :widths: 25 25 50&#10;</xsl:text>
        <xsl:text>    :header-rows: 1&#10;&#10;</xsl:text>
        <xsl:text>    * - Element&#10;</xsl:text>
        <xsl:text>      - Type&#10;</xsl:text>
        <xsl:text>      - Description&#10;</xsl:text>-->
        <xsl:text>Elements&#10;--------&#10;&#10;</xsl:text>
        <xsl:apply-templates select="xsd:group" mode="rstdoc-table"/>
        <xsl:apply-templates select="xsd:element" mode="rstdocelement-table"/>
        <xsl:apply-templates select="xsd:choice/xsd:element" mode="rstdocelement-table"/>
        <xsl:text>&#10;</xsl:text>
    </xsl:template>
    <xsl:template match="xsd:choice" mode="rstdoc-table">
        <!--
        <xsl:text>.. list-table::&#10;</xsl:text>
        <xsl:text>    :widths: 25 25 50&#10;</xsl:text>
        <xsl:text>    :header-rows: 1&#10;&#10;</xsl:text>
        <xsl:text>    * - Element&#10;</xsl:text>
        <xsl:text>      - Type&#10;</xsl:text>
        <xsl:text>      - Description&#10;</xsl:text>-->
        <xsl:text>Elements&#10;--------&#10;&#10;</xsl:text>
        <xsl:apply-templates select="xsd:element" mode="rstdocelement-table"/>
        <xsl:text>&#10;</xsl:text>
    </xsl:template>
    <xsl:template match="xsd:group" mode="rstdoc-table">
        <!--<xsl:text>  Group </xsl:text><xsl:value-of select="@ref"/><xsl:text>&#10;</xsl:text>-->
        <xsl:variable name="groupname" select="@ref"/>
        <xsl:variable name="groupnode" select="//xsd:group[@name=$groupname]" />
        <!--<xsl:value-of select="$groupnode"/>-->
        <!--<xsl:value-of select="$groupnode/@name"/>-->
        <xsl:apply-templates select="$groupnode" mode="rstdocelements-table"/>
    </xsl:template>
    <xsl:template match="xsd:group" mode="rstdocelements-table">
        <xsl:apply-templates select="xsd:sequence/xsd:element" mode="rstdocelement-table"/>
    </xsl:template>
    <!--
    <xsl:template match="xsd:element" mode="rstdocelement-table">
        <xsl:text>    * - ``</xsl:text><xsl:value-of select="@name"/><xsl:text>``&#10;</xsl:text>
        <xsl:text>      - </xsl:text>
        <xsl:call-template name="type-reference">
            <xsl:with-param name="tname" select="@type"/>
        </xsl:call-template>
        <xsl:text>&#10;</xsl:text>        
        <xsl:text>      - </xsl:text>
        <xsl:apply-templates select="./xsd:annotation/xsd:documentation" mode="rstdoc"/>
        <xsl:text>&#10;</xsl:text>
    </xsl:template>-->
    
    <xsl:template match="xsd:element" mode="rstdocelement-table">
        <xsl:text>``</xsl:text><xsl:value-of select="@name"/><xsl:text>``: </xsl:text> 
        <xsl:call-template name="type-reference">
            <xsl:with-param name="tname" select="@type"/>
        </xsl:call-template>
        <xsl:text>&#10;&#9;</xsl:text>
        <xsl:apply-templates select="./xsd:annotation/xsd:documentation" mode="rstdoc"/>
        <xsl:text>&#10;&#10;</xsl:text>
    </xsl:template>
    <xsl:template match="xsd:extension" mode="rstdoc-table">
        <xsl:text>Extends: </xsl:text>
        <xsl:call-template name="type-reference">
            <xsl:with-param name="tname" select="@base"/>
        </xsl:call-template>
        <xsl:text>&#10;</xsl:text>        
        <xsl:text>&#10;</xsl:text>
        <xsl:apply-templates select="./xsd:sequence" mode="rstdoc-xml"/>
        <xsl:apply-templates select="./xsd:choice" mode="rstdoc-xml"/>
        <xsl:apply-templates select="./xsd:sequence" mode="rstdoc-table"/>
        <xsl:apply-templates select="./xsd:choice" mode="rstdoc-table"/>
        <xsl:if test="count(xsd:attribute) &gt; 0">
        <!--    
            <xsl:text>.. list-table::&#10;</xsl:text>
            <xsl:text>    :widths: 25 25 50&#10;</xsl:text>
            <xsl:text>    :header-rows: 1&#10;&#10;</xsl:text>
            <xsl:text>    * - Attribute&#10;</xsl:text>
            <xsl:text>      - Type&#10;</xsl:text>
            <xsl:text>      - Description&#10;</xsl:text>-->
            <xsl:text>Attributes&#10;-----------&#10;&#10;</xsl:text>
            <xsl:apply-templates select="xsd:attribute" mode="rstdocattr-table"/>
            <xsl:text>&#10;</xsl:text>
        </xsl:if>
            <xsl:if test="count(xsd:enumeration) &gt; 0">
            <xsl:text>Allowed values:&#10;&#10;</xsl:text>
            <xsl:apply-templates select="xsd:enumeration" mode="rstdocenum-list"/>
            <xsl:text>&#10;</xsl:text>
        </xsl:if>
        <xsl:if test="count(xsd:pattern) &gt; 0">
            <xsl:text>Allowed patterns:&#10;&#10;</xsl:text>
            <xsl:apply-templates select="xsd:pattern" mode="rstdocenum-list"/>
            <xsl:text>&#10;</xsl:text>
        </xsl:if>
    </xsl:template>
    <xsl:template match="xsd:restriction" mode="rstdoc-table">
        <xsl:text>Restricts: </xsl:text>
        <xsl:call-template name="type-reference">
            <xsl:with-param name="tname" select="@base"/>
        </xsl:call-template>
        <xsl:text>&#10;</xsl:text>
        <xsl:text>&#10;</xsl:text>
        <xsl:apply-templates select="./xsd:sequence" mode="rstdoc-xml"/>
        <xsl:apply-templates select="./xsd:choice" mode="rstdoc-xml"/>
        <xsl:apply-templates select="./xsd:sequence" mode="rstdoc-table"/>
        <xsl:apply-templates select="./xsd:choice" mode="rstdoc-table"/>
        <xsl:if test="count(xsd:attribute) &gt; 0">
            <!--
            <xsl:text>.. list-table::&#10;</xsl:text>
            <xsl:text>    :widths: 25 25 50&#10;</xsl:text>
            <xsl:text>    :header-rows: 1&#10;&#10;</xsl:text>
            <xsl:text>    * - Attribute&#10;</xsl:text>
            <xsl:text>      - Type&#10;</xsl:text>
            <xsl:text>      - Description&#10;</xsl:text>-->
            <xsl:text>Attributes&#10;-----------&#10;&#10;</xsl:text>
            <xsl:apply-templates select="xsd:attribute" mode="rstdocattr-table"/>
            <xsl:text>&#10;</xsl:text>
        </xsl:if>
        <xsl:if test="count(xsd:enumeration) &gt; 0">
            <xsl:text>Allowed values:&#10;&#10;</xsl:text>
            <xsl:apply-templates select="xsd:enumeration" mode="rstdocenum-list"/>
            <xsl:text>&#10;</xsl:text>
        </xsl:if>
        <xsl:if test="count(xsd:pattern) &gt; 0">
            <xsl:text>Allowed patterns:&#10;&#10;</xsl:text>
            <xsl:apply-templates select="xsd:pattern" mode="rstdocenum-list"/>
            <xsl:text>&#10;</xsl:text>
        </xsl:if>
    </xsl:template>
    <xsl:template match="xsd:attribute[@type]" mode="rstdocattr-table">
        <!--
        <xsl:text>    * - ``</xsl:text><xsl:value-of select="@name"/><xsl:text>``&#10;</xsl:text>
        <xsl:text>      - ``</xsl:text>
        <xsl:apply-templates select="." mode="use"/>
        <xsl:text>`` </xsl:text>
        <xsl:call-template name="type-reference">
            <xsl:with-param name="tname" select="@type"/>
        </xsl:call-template>
        <xsl:text>&#10;</xsl:text>        
        <xsl:text>      - </xsl:text>
        <xsl:apply-templates select="./xsd:annotation/xsd:documentation" mode="rstdoc"/>
        <xsl:text>&#10;</xsl:text>
        -->
        <xsl:text>``</xsl:text><xsl:value-of select="@name"/><xsl:text>``: </xsl:text>
        <xsl:text>``</xsl:text>
        <xsl:apply-templates select="." mode="use"/>
        <xsl:text>`` </xsl:text>
        <xsl:call-template name="type-reference">
            <xsl:with-param name="tname" select="@type"/>
        </xsl:call-template>
        <xsl:text>&#10;&#9;</xsl:text>
        <xsl:apply-templates select="./xsd:annotation/xsd:documentation" mode="rstdoc"/>
        <xsl:text>&#10;&#10;</xsl:text>
    </xsl:template>
    <xsl:template match="xsd:attribute[not(@type)]" mode="rstdocattr-table">
        <!--
        <xsl:text>    * - ``</xsl:text><xsl:value-of select="@name"/><xsl:text>``&#10;</xsl:text>
        <xsl:text>      - ``</xsl:text>
        <xsl:apply-templates select="." mode="use"/>
        <xsl:text>`` </xsl:text>
        <xsl:call-template name="type-reference">
            <xsl:with-param name="tname" select="'xs:string'"/>
        </xsl:call-template>
        <xsl:text>&#10;</xsl:text>        
        <xsl:text>      - </xsl:text>
        <xsl:apply-templates select="./xsd:annotation/xsd:documentation" mode="rstdoc"/>
        <xsl:text>&#10;</xsl:text>
        -->
        <xsl:text>``</xsl:text><xsl:value-of select="@name"/><xsl:text>``: </xsl:text>
        <xsl:text>``</xsl:text>
        <xsl:apply-templates select="." mode="use"/>
        <xsl:text>`` </xsl:text>
        <xsl:call-template name="type-reference">
            <xsl:with-param name="tname" select="'xs:string'"/>
        </xsl:call-template>
        <xsl:text>&#10;&#9;</xsl:text>
        <xsl:apply-templates select="./xsd:annotation/xsd:documentation" mode="rstdoc"/>
        <xsl:text>&#10;&#10;</xsl:text>
    </xsl:template>
    <xsl:template match="xsd:enumeration|xsd:pattern[substring(@value, 1, 4)!='http']" mode="rstdocenum-list">
        <xsl:text>- ``</xsl:text>
        <xsl:value-of select="@value"/>
        <xsl:text>``&#10;</xsl:text>        
    </xsl:template>
    <xsl:template match="xsd:enumeration|xsd:pattern[substring(@value, 1, 4)='http']" mode="rstdocenum-list">
        <xsl:text>- `</xsl:text>
        <xsl:value-of select="@value"/>
        <xsl:text> &lt;</xsl:text>
        <xsl:value-of select="@value"/>
        <xsl:text>&gt;`_&#10;</xsl:text>        
    </xsl:template>
    <xsl:template match="xsd:union">
        <xsl:text>Union: </xsl:text>
        <xsl:call-template name="tokenize-references">
            <xsl:with-param name="string" select="@memberTypes" />
        </xsl:call-template>
        <xsl:text>&#10;</xsl:text>
        <xsl:text>&#10;</xsl:text>
    </xsl:template>
    <xsl:template name="type-reference">
        <xsl:param name="tname"/>
        <xsl:variable name="norm-type">
            <xsl:call-template name="type-name">
                <xsl:with-param name="tname" select="$tname"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:choose>
            <xsl:when test="substring($tname, 1, 3) = 'xs:'">
                <!-- xsd:string <https://www.w3.org/TR/xmlschema11-2/#string>`_ -->
                <xsl:text>`</xsl:text>
                <xsl:value-of select="$tname"/>
                <xsl:text> &lt;</xsl:text>
                <xsl:text>https://www.w3.org/TR/xmlschema11-2/#</xsl:text>
                <xsl:value-of select="substring-after($tname, 'xs:')"/>
                <xsl:text>&gt;`_</xsl:text>
            </xsl:when>
            <xsl:otherwise>
                <!-- :ref:`identifier-type` -->
                <xsl:text>:ref:`</xsl:text>
                <xsl:value-of select="translate($norm-type, $uppercase, $lowercase)"/>
                <xsl:text>-type`</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="type-name">
        <xsl:param name="tname"/>
        <xsl:choose>
            <xsl:when test="contains($tname, 'Type')">
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
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="$tname"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template match="xsd:attribute" mode="use">
        <xsl:choose>
            <xsl:when test="@use">
                <xsl:value-of select="@use"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text>optional</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template match="xsd:element|xsd:choice" mode="min">
        <xsl:choose>
            <xsl:when test="@minOccurs">
                <xsl:value-of select="@minOccurs"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text>1</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template match="xsd:element|xsd:choice" mode="max">
        <xsl:choose>
            <xsl:when test="@maxOccurs = 'unbounded'">
                <xsl:text>n</xsl:text>
            </xsl:when>
            <xsl:when test="@maxOccurs != 'unbounded'">
                <xsl:value-of select="@maxOccurs"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text>1</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="underline">
        <xsl:param name="item" select="'='"/>
        <xsl:param name="count" />
        <xsl:if test="$count &gt; 0">
            <xsl:value-of select="$item" />
            <xsl:call-template name="underline">
                <xsl:with-param name="item" select="$item" />
                <xsl:with-param name="count" select="$count - 1" />
            </xsl:call-template>
        </xsl:if>
    </xsl:template>
    <xsl:template name="tokenize-references">
        <xsl:param name="string"/>
        <xsl:choose>
            <xsl:when test="contains($string,' ')">
                <xsl:call-template name="type-reference">
                    <xsl:with-param name="tname" select="substring-before($string,' ')"/>
                </xsl:call-template>
                <xsl:text> </xsl:text>
                <xsl:call-template name="tokenize-references">
                    <xsl:with-param name="string" select="substring-after($string,' ')"/>
                </xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
                <xsl:call-template name="type-reference">
                    <xsl:with-param name="tname" select="$string"/>
                </xsl:call-template>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template match="text()" />
</xsl:stylesheet>