<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:xsd="http://www.w3.org/2001/XMLSchema" version="1.0">
    <xsl:output method="xml" indent="yes" encoding="utf-8"/>
    <xsl:param name="action" select="'list-types'"/>

    <xsl:template match="/">
        <xsl:choose>
            <xsl:when test="$action = 'structure'">
                <xsl:apply-templates select="/xsd:schema/xsd:element" mode="structure"/>
		    </xsl:when>
            <xsl:when test="$action = 'list-types'">
                <xsl:call-template name="list-types"/>
		    </xsl:when>
	    </xsl:choose>
    </xsl:template>

    <!--
        Structure
    -->

    <xsl:template match="xsd:element" mode="structure">
    	<xsl:variable name="min"><xsl:apply-templates select="." mode="min"/></xsl:variable>
    	<xsl:variable name="max"><xsl:apply-templates select="." mode="max"/></xsl:variable>
    	<xsl:element name="{@name}_{$min}_{$max}">
    		<!--<xsl:attribute name="name"><xsl:value-of select="@name"/></xsl:attribute>-->
    		<!--
    		<xsl:attribute name="_rep_">
    			<xsl:apply-templates select="." mode="min"/>
	    		<xsl:text>,</xsl:text>
		    	<xsl:apply-templates select="." mode="max"/>
    		</xsl:attribute>-->
    		<xsl:variable name="tname" select="@type"/>
    		<xsl:variable name="typenode" select="//xsd:complexType[@name=$tname]|//xsl:simpleType[@name=$tname]" />
			<xsl:for-each select="$typenode/xsd:attribute">
    			<xsl:attribute name="{@name}">
    				<xsl:text>_</xsl:text>
    				<xsl:value-of select="@use"/>
    				<xsl:text>_</xsl:text>
    			</xsl:attribute>
    		</xsl:for-each>
    		<xsl:choose>
				<xsl:when test="count(//xsd:element[@type=$tname]) = 1 and count(//xsd:attribute[@type=$tname]) = 0 and count(//xsd:extension[@base=$tname]) = 0 and count(//xsd:restriction[@base=$tname]) = 0">
	    			<xsl:apply-templates select="$typenode" mode="structure"/>
		    	</xsl:when>
		    	<xsl:otherwise>
		    		<xsl:text>...</xsl:text><xsl:value-of select="$tname"/><xsl:text>...</xsl:text>
		    	</xsl:otherwise>
		    </xsl:choose>
    	</xsl:element>
    </xsl:template>

    <xsl:template match="xsd:complexType" mode="structure">
    	<xsl:apply-templates select="*" mode="structure" />
    </xsl:template>

    <xsl:template match="xsd:sequence" mode="structure">
    	<xsl:apply-templates select="*" mode="structure" />
    </xsl:template>

    <xsl:template match="xsd:choice" mode="structure">
    	<xsl:comment>
	    	<xsl:text> choice </xsl:text>
	    	<xsl:apply-templates select="." mode="min"/>
	    	<xsl:text>,</xsl:text>
	    	<xsl:apply-templates select="." mode="max"/>
	    	<xsl:text> </xsl:text>
    	</xsl:comment>
    	<xsl:apply-templates select="*" mode="structure" />
    	<xsl:comment>
    		<xsl:text> /choice </xsl:text>
		</xsl:comment>
    </xsl:template>

    <xsl:template match="xsd:group[@ref]" mode="structure">
    	<xsl:variable name="group" select="@ref"/>
    	<xsl:variable name="groupnode" select="//xsd:group[@name=$group]" />
    	<xsl:comment>
    		<xsl:text> group </xsl:text>
    		<xsl:value-of select="$group"/>
    	</xsl:comment>
    	<xsl:apply-templates select="$groupnode" mode="structure" />
    	<xsl:comment>
    		<xsl:text> /group </xsl:text>
    		<xsl:value-of select="$group"/>
    	</xsl:comment>
    </xsl:template>

    <xsl:template match="xsd:group[@name]" mode="structure">
    	<xsl:apply-templates select="*" mode="structure" />
    </xsl:template>

    <xsl:template match="xsd:restriction|xsd:extension" mode="structure">
    	<xsl:variable name="tname" select="@base"/>
    	<xsl:variable name="typenode" select="//xsd:complexType[@name=$tname]|//xsl:simpleType[@name=$tname]" />
    	<!--<xsl:for-each select="$typenode/xsd:attribute">
    		<xsl:attribute name="{@name}"><xsl:text>1</xsl:text></xsl:attribute>
    	</xsl:for-each>-->
    	<xsl:for-each select="xsd:attribute">
    		<xsl:if test="@use != 'prohibited'">
	    		<xsl:attribute name="{@name}"><xsl:value-of select="@use"/></xsl:attribute>
	    	</xsl:if>
    	</xsl:for-each>
    	<xsl:apply-templates select="$typenode" mode="structure" />
    </xsl:template>

    <xsl:template match="text()|@*" mode="structure"/>

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

    <xsl:template name="list-types">
    	<!-- Mono-element types -->
    	<xsl:text>Mono-element complex types: &#10;</xsl:text>
    	<xsl:for-each select="//xsd:complexType[@name]">
    		<xsl:variable name="tname" select="@name"/>
    		<xsl:if test="count(//xsd:element[@type=$tname]) = 1 and count(//xsd:attribute[@type=$tname]) = 0 and count(//xsd:extension[@base=$tname]) = 0 and count(//xsd:restriction[@base=$tname]) = 0">
    			<xsl:text>    </xsl:text>
    			<xsl:value-of select="@name" />
    			<xsl:text>&#10;</xsl:text>
    		</xsl:if>
    	</xsl:for-each>
    	<xsl:text>Mono-element simple types: &#10;</xsl:text>
    	<xsl:for-each select="//xsd:simpleType[@name]">
    		<xsl:variable name="tname" select="@name"/>
    		<xsl:if test="count(//xsd:element[@type=$tname]) = 1 and count(//xsd:attribute[@type=$tname]) = 0 and count(//xsd:extension[@base=$tname]) = 0 and count(//xsd:restriction[@base=$tname]) = 0">
    			<xsl:text>    </xsl:text>
    			<xsl:value-of select="@name" />
    			<xsl:text>&#10;</xsl:text>
    		</xsl:if>
    	</xsl:for-each>
    	<!-- Multi-elements types -->
    	<xsl:text>Multi-element complex types: &#10;</xsl:text>
    	<xsl:for-each select="//xsd:complexType[@name]">
    		<xsl:variable name="tname" select="@name"/>
    		<xsl:if test="count(//xsd:element[@type=$tname]) &gt; 1 and count(//xsd:attribute[@type=$tname]) = 0">
    			<xsl:text>    </xsl:text>
    			<xsl:value-of select="@name" />
    			<xsl:text>&#10;</xsl:text>
    		</xsl:if>
    	</xsl:for-each>
    	<xsl:text>Multi-element simple types: &#10;</xsl:text>
    	<xsl:for-each select="//xsd:simpleType[@name]">
    		<xsl:variable name="tname" select="@name"/>
    		<xsl:if test="count(//xsd:element[@type=$tname]) &gt; 1 and count(//xsd:attribute[@type=$tname]) = 0">
    			<xsl:text>    </xsl:text>
    			<xsl:value-of select="@name" />
    			<xsl:text>&#10;</xsl:text>
    		</xsl:if>
    	</xsl:for-each>
    	<!-- Unused types -->
    	<xsl:text>Unused complex types: &#10;</xsl:text>
    	<xsl:for-each select="//xsd:complexType[@name]">
    		<xsl:variable name="tname" select="@name"/>
    		<xsl:if test="count(//xsd:element[@type=$tname]) = 0 and count(//xsd:attribute[@type=$tname]) = 0 and count(//xsd:extension[@base=$tname]) = 0 and count(//xsd:restriction[@base=$tname]) = 0">
    			<xsl:text>    </xsl:text>
    			<xsl:value-of select="@name" />
    			<xsl:text>&#10;</xsl:text>
    		</xsl:if>
    	</xsl:for-each>
    	<!-- Base types -->
    	<xsl:text>Base complex types: &#10;</xsl:text>
    	<xsl:for-each select="//xsd:complexType[@name]">
    		<xsl:variable name="tname" select="@name"/>
    		<xsl:if test="count(//xsd:extension[@base=$tname]) &gt; 0 or count(//xsd:restriction[@base=$tname]) &gt; 0">
    			<xsl:text>    </xsl:text>
    			<xsl:value-of select="@name" />
    			<xsl:text>&#10;</xsl:text>
    		</xsl:if>
    	</xsl:for-each>
    	<xsl:text>Base simple types: &#10;</xsl:text>
    	<xsl:for-each select="//xsd:simpleType[@name]">
    		<xsl:variable name="tname" select="@name"/>
    		<xsl:if test="count(//xsd:extension[@base=$tname]) &gt; 0 or count(//xsd:restriction[@base=$tname]) &gt; 0">
    			<xsl:text>    </xsl:text>
    			<xsl:value-of select="@name" />
    			<xsl:text>&#10;</xsl:text>
    		</xsl:if>
    	</xsl:for-each>
    	<!-- Groups -->
    	<xsl:text>Groups: &#10;</xsl:text>
    	<xsl:for-each select="//xsd:group[@name]">
    		<xsl:variable name="tname" select="@name"/>
    		<xsl:if test="count(//xsd:sequence[xsd:group[@ref=$tname]]) &gt; 0">
    			<xsl:text>    </xsl:text>
    			<xsl:value-of select="@name" />
    			<xsl:text>&#10;</xsl:text>
    		</xsl:if>
    	</xsl:for-each>
    	<!-- Attributes -->
	   	<xsl:text>Attributes complex types: &#10;</xsl:text>
    	<xsl:for-each select="//xsd:complexType[@name]">
    		<xsl:variable name="tname" select="@name"/>
    		<xsl:if test="count(//xsd:element[@type=$tname]) = 0 and count(//xsd:attribute[@type=$tname]) &gt; 0">
    			<xsl:text>    </xsl:text>
    			<xsl:value-of select="@name" />
    			<xsl:text>&#10;</xsl:text>
    		</xsl:if>
    	</xsl:for-each>
    	<xsl:text>Attributes simple types: &#10;</xsl:text>
    	<xsl:for-each select="//xsd:simpleType[@name]">
    		<xsl:variable name="tname" select="@name"/>
    		<xsl:if test="count(//xsd:element[@type=$tname]) = 0 and count(//xsd:attribute[@type=$tname]) &gt; 0">
    			<xsl:text>    </xsl:text>
    			<xsl:value-of select="@name" />
    			<xsl:text>&#10;</xsl:text>
    		</xsl:if>
    	</xsl:for-each>

    </xsl:template>

    <!--
    <xsl:template match="//xsd:group[@name]">
    	<xsl:variable name="tname" select="@name"/>
    	<xsl:text>====== Group: </xsl:text>
        <xsl:value-of select="$tname" />
        <xsl:text> </xsl:text>
        <xsl:text>Elements: </xsl:text>
        <xsl:value-of select="count(//xsd:sequence[xsd:group[@ref=$tname]])"/>
        <xsl:text>&#10;</xsl:text>
    </xsl:template>
    <xsl:template match="//xsd:complexType[@name]">
    	<xsl:variable name="tname" select="@name"/>
    	<xsl:text>====== ComplexType: </xsl:text>
        <xsl:value-of select="$tname" />
        <xsl:text> </xsl:text>
        <xsl:text>Elements: </xsl:text>
        <xsl:value-of select="count(//xsd:element[@type=$tname])"/>
        <xsl:text> Attributes: </xsl:text>
        <xsl:value-of select="count(//xsd:attribute[@type=$tname])"/>
        <xsl:text> Extensions: </xsl:text>
        <xsl:value-of select="count(//xsd:extension[@base=$tname])"/>
        <xsl:text> Restrictions: </xsl:text>
        <xsl:value-of select="count(//xsd:restriction[@base=$tname])"/>
        <xsl:text>&#10;</xsl:text>
    </xsl:template>
    <xsl:template match="//xsd:simpleType[@name]">
    	<xsl:variable name="tname" select="@name"/>
    	<xsl:text>====== SimpleType: </xsl:text>
        <xsl:value-of select="$tname" />
        <xsl:text> </xsl:text>
        <xsl:text>Elements: </xsl:text>
        <xsl:value-of select="count(//xsd:element[@type=$tname])"/>
        <xsl:text> Attributes: </xsl:text>
        <xsl:value-of select="count(//xsd:attribute[@type=$tname])"/>
        <xsl:text> Extensions: </xsl:text>
        <xsl:value-of select="count(//xsd:extension[@base=$tname])"/>
        <xsl:text> Restrictions: </xsl:text>
        <xsl:value-of select="count(//xsd:restriction[@base=$tname])"/>
        <xsl:text>&#10;</xsl:text>
    </xsl:template>-->
    <xsl:template match="text()" />
</xsl:stylesheet>