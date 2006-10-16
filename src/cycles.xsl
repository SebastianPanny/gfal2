<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
    <!-- FileName: CycleFind.xsl -->
    <!-- Creator: David Marston -->
    <!-- Purpose: Find cycles by moving nodes from Unvisited to Pending to
Finished.
Input must have node identifier (@ni) and connector identifier (@ci)
attributes. -->
    <xsl:output method="xml" encoding="UTF-8" indent="yes"/>
    <xsl:template match="WFBPR">
        <!-- the document element -->
        <WFBPR>
            <xsl:copy-of select="RepositoryData"/>
            <xsl:apply-templates select="Processes"/>
        </WFBPR>
    </xsl:template>
    <xsl:template match="Processes">
        <Processes>
            <xsl:apply-templates select="Process"/>
        </Processes>
    </xsl:template>
    <xsl:template match="Process">
        <!-- simple copying continues until we reach the Connectors element
-->
        <Process>
            <xsl:copy-of select="ProcessName"/>
            <xsl:copy-of select="ProcessType"/>
            <xsl:copy-of select="TaskObjects"/>
            <xsl:copy-of select="PhiObjects"/>
            <xsl:copy-of select="ExternalProcessObjects"/>
            <xsl:copy-of select="DecisionObjects"/>
            <Connectors>
                <!-- Now walk through the graph. Start by finding all the starting
nodes. -->
                <xsl:variable name="Starts"
                    select="ExternalProcessObjects/ExternalProcessObject[@Start]/@ni"/>
                <xsl:if test="count($Starts) = 0">
                    <xsl:message terminate="yes">
                        <xsl:text>No starting nodes found!</xsl:text>
                    </xsl:message>
                </xsl:if>
                <!-- Convert the node-set ($Starts) to a list, which will begin
our stack. -->
                <xsl:variable name="StartList">
                    <xsl:call-template name="MakeStart">
                        <xsl:with-param name="list" select="$Starts"/>
                        <xsl:with-param name="lSeq" select="count($Starts)"/>
                        <xsl:with-param name="work" select="'+'"/>
                        <!-- empty list -->
                    </xsl:call-template>
                </xsl:variable>
                <!-- Now pop off the first member of the start list as the
starting node. The rest go into $Stack. -->
                <xsl:variable name="begin" select="substring-before($StartList,'+')"/>
                <!-- The goal of the recursive stepping is to populate the list in
$BackLInks. -->
                <xsl:variable name="BackLinks">
                    <!-- Now make the initial call to Step. -->
                    <xsl:call-template name="Step">
                        <xsl:with-param name="Node" select="$begin"/>
                        <xsl:with-param name="Pending" select="$begin"/>
                        <!-- this node
will be in Pending state when we go in -->
                        <xsl:with-param name="Finished" select="''"/>
                        <!-- empty list
-->
                        <xsl:with-param name="UsedConn" select="''"/>
                        <!-- empty list
-->
                        <xsl:with-param name="BackList" select="''"/>
                        <!-- empty list
-->
                        <xsl:with-param name="Stack" select="substring-after($StartList,'+')"/>
                    </xsl:call-template>
                </xsl:variable>
                <!-- At this point, we have detected all back-links and put them
on the $BackLInks list. -->
                <!-- Copy each Connector straight through unless flagged as a
back-link. -->
                <xsl:for-each select="Connectors/Connector">
                    <xsl:choose>
                        <xsl:when test="contains($BackLinks,@ci)">
                            <!-- This Connector is a back-link. Put a special attribute
to mark that fact. -->
                            <Connector BackLink="yes">
                                <!-- then copy all other attributes and descendants -->
                                <xsl:copy-of select="./@*"/>
                                <xsl:copy-of select="./node()"/>
                            </Connector>
                        </xsl:when>
                        <xsl:otherwise>
                            <!-- It's a normal connector, so just copy it through. -->
                            <xsl:copy-of select="."/>
                        </xsl:otherwise>
                    </xsl:choose>
                </xsl:for-each>
            </Connectors>
        </Process>
    </xsl:template>
    <!-- Named templates below, alphabetcally by name. -->
    <xsl:template name="FindUnused">
        <!-- Look for unused connector in $conns list. Return ci of unused or
"!NONE!". -->
        <xsl:param name="conns"/>
        <xsl:param name="fSeq"/>
        <xsl:param name="used"/>
        <!-- A copy of the latest list of all
connectors used so far. -->
        <xsl:choose>
            <xsl:when test="contains($used,$conns[$fSeq]/@ci)">
                <!-- This one is already used. Are there more? -->
                <xsl:choose>
                    <xsl:when test="$fSeq=1">
                        <xsl:text>!NONE!</xsl:text>
                    </xsl:when>
                    <xsl:otherwise>
                        <!-- There are more, so recurse with a lower $fseq number. -->
                        <xsl:call-template name="FindUnused">
                            <xsl:with-param name="conns" select="$conns"/>
                            <xsl:with-param name="fSeq" select="$fSeq - 1"/>
                            <xsl:with-param name="used" select="$used"/>
                        </xsl:call-template>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:when>
            <xsl:otherwise>
                <!-- "Return" the @ci of this connector as the next
unused one. -->
                <xsl:value-of select="$conns[$fSeq]/@ci"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="MakeStart">
        <!-- Take members of node-set $list and form a delimited concatenation
of their string values. -->
        <xsl:param name="list"/>
        <xsl:param name="lSeq"/>
        <xsl:param name="work"/>
        <xsl:choose>
            <xsl:when test="$lSeq=1">
                <!-- Down to the last one. Finish off the string and "return" it.
-->
                <xsl:value-of select="concat(string($list[1]),$work)"/>
            </xsl:when>
            <xsl:otherwise>
                <!-- Add the current member (and a delimiter) to the string, then
recurse. -->
                <xsl:call-template name="MakeStart">
                    <xsl:with-param name="list" select="$list"/>
                    <!-- Don't bother
shrinking this. -->
                    <xsl:with-param name="lSeq" select="$lSeq - 1"/>
                    <xsl:with-param name="work"
                        select="concat('+',string($list[position()=$lSeq]),$work)"/>
                </xsl:call-template>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:template name="Step">
        <!-- Step from one Node to another, or to the same Node with updated
data. This is a 5-branch decision tree. -->
        <xsl:param name="Node"/>
        <!-- ni of the Node we are stepping to -->
        <xsl:param name="Pending"/>
        <!-- string containing ni of all Nodes
(including current $Node) visited but not done -->
        <xsl:param name="Finished"/>
        <!-- string containing ni of all Nodes
visited and resolved -->
        <xsl:param name="UsedConn"/>
        <!-- string containing ci of all
connectors that have been analyzed -->
        <xsl:param name="BackList"/>
        <!-- string containing ci of all
connectors identified as back-links so far -->
        <xsl:param name="Stack"/>
        <!-- string containing ni of Nodes on the
path to here (excluding $Node) -->
        <xsl:message>
            <xsl:text>-----STEP into box </xsl:text>
            <xsl:value-of select="$Node"/>
        </xsl:message>
        <!-- List and count all outbound links from here. -->
        <xsl:variable name="ConnList" select="Connectors/Connector[Source/@li=$Node]"/>
        <xsl:variable name="TotalC" select="count($ConnList)"/>
        <xsl:choose>
            <xsl:when test="$TotalC = 0">
                <!-- No outbound links, so this ends a path. -->
                <xsl:choose>
                    <xsl:when test="string-length($Stack) &gt; 0">
                        <!-- More work to do. -->
                        <xsl:call-template name="Step">
                            <xsl:with-param name="Node" select="substring-before($Stack,'+')"/>
                            <!-- Get "next" node off the
stack. -->
                            <xsl:with-param name="Pending"
                                select="concat(substring-before($Pending,$Node),
substring-after($Pending,concat($Node,'+')))"/>
                            <!--
Current node no longer pending. -->
                            <xsl:with-param name="Finished" select="concat($Node,'+',$Finished)"/>
                            <!-- Add current node to Finished
list. -->
                            <xsl:with-param name="UsedConn" select="$UsedConn"/>
                            <!-- We
are backing up; no additional usage. -->
                            <xsl:with-param name="BackList" select="$BackList"/>
                            <xsl:with-param name="Stack" select="substring-after($Stack,'+')"/>
                            <!-- This is the rest of the
stack. -->
                        </xsl:call-template>
                    </xsl:when>
                    <xsl:otherwise>
                        <!-- The stack is empty; "Return" the list of back-links. -->
                        <xsl:value-of select="$BackList"/>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:when>
            <xsl:otherwise>
                <!-- This node has some outbound links. Look for unused ones. -->
                <xsl:variable name="NextUnused">
                    <xsl:call-template name="FindUnused">
                        <xsl:with-param name="conns" select="$ConnList"/>
                        <xsl:with-param name="fSeq" select="$TotalC"/>
                        <xsl:with-param name="used" select="$UsedConn"/>
                        <!-- Must give
it the current snapshot of what's used. -->
                    </xsl:call-template>
                </xsl:variable>
                <xsl:choose>
                    <xsl:when test="$NextUnused = '!NONE!'">
                        <!-- All outbound connectors have been traversed. -->
                        <xsl:choose>
                            <xsl:when test="string-length($Stack) &gt; 0">
                                <!-- More work to do. -->
                                <xsl:call-template name="Step">
                                    <xsl:with-param name="Node"
                                        select="substring-before($Stack,'+')"/>
                                    <!-- Get "next" node off the
stack. -->
                                    <xsl:with-param name="Pending"
                                        select="concat(substring-before($Pending,$Node),
substring-after($Pending,concat($Node,'+')))"/>
                                    <!--
Current node no longer pending. -->
                                    <xsl:with-param name="Finished"
                                        select="concat($Node,'+',$Finished)"/>
                                    <!-- Add current node to Finished
list. -->
                                    <xsl:with-param name="UsedConn" select="$UsedConn"/>
                                    <!--
We are backing up; no additional usage. -->
                                    <xsl:with-param name="BackList" select="$BackList"/>
                                    <xsl:with-param name="Stack"
                                        select="substring-after($Stack,'+')"/>
                                    <!-- This is the rest of the
stack. -->
                                </xsl:call-template>
                            </xsl:when>
                            <xsl:otherwise>
                                <!-- The stack is empty; "Return" the list of back-links.
-->
                                <xsl:value-of select="$BackList"/>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:when>
                    <xsl:otherwise>
                        <!-- We have an unused connector to follow. There are three
possibilities:
A) Connects to a Node that has been finished. No problem
here.
B) Connects to a Node whose descendants are still being
explored. This is a back-link.
C) Connects to a Node that has not been visited before. Step
into it and keep looking. -->
                        <xsl:variable name="Follow"
                            select="Connectors/Connector[@ci=$NextUnused]/Target/TargetName/@li"/>
                        <!-- $Follow is the identifier (ni) of the node on the other
end of connector $NextUnused. -->
                        <xsl:choose>
                            <xsl:when test="contains($Finished,$Follow)">
                                <!-- Case A -->
                                <!-- Now stay on $Node, but mark current connector used,
which requires a Step call. -->
                                <xsl:call-template name="Step">
                                    <xsl:with-param name="Node" select="$Node"/>
                                    <xsl:with-param name="Pending" select="$Pending"/>
                                    <xsl:with-param name="Finished" select="$Finished"/>
                                    <xsl:with-param name="UsedConn"
                                        select="concat($NextUnused,'+',$UsedConn)"/>
                                    <!-- Add to list. -->
                                    <xsl:with-param name="BackList" select="$BackList"/>
                                    <xsl:with-param name="Stack" select="$Stack"/>
                                </xsl:call-template>
                            </xsl:when>
                            <xsl:when test="contains($Pending,$Follow)">
                                <!-- Case B:
found a back-link -->
                                <!-- Now stay on $Node, but mark current connector used,
which requires a Step call. -->
                                <xsl:call-template name="Step">
                                    <xsl:with-param name="Node" select="$Node"/>
                                    <xsl:with-param name="Pending" select="$Pending"/>
                                    <xsl:with-param name="Finished" select="$Finished"/>
                                    <xsl:with-param name="UsedConn"
                                        select="concat($NextUnused,'+',$UsedConn)"/>
                                    <!-- Add to list. -->
                                    <xsl:with-param name="BackList"
                                        select="concat($NextUnused,'+',$BackList)"/>
                                    <!-- Also add to this list.
-->
                                    <xsl:with-param name="Stack" select="$Stack"/>
                                </xsl:call-template>
                            </xsl:when>
                            <xsl:otherwise>
                                <!-- Case C -->
                                <!-- Follow the connector. This is the conventional
forward step. -->
                                <xsl:call-template name="Step">
                                    <xsl:with-param name="Node" select="$Follow"/>
                                    <xsl:with-param name="Pending"
                                        select="concat($Follow,'+',$Pending)"/>
                                    <!-- This is our first visit to the node. Update
$Pending now. -->
                                    <xsl:with-param name="Finished" select="$Finished"/>
                                    <xsl:with-param name="UsedConn"
                                        select="concat($NextUnused,'+',$UsedConn)"/>
                                    <!-- Update $UsedConn also, because we are using a
previously-unused connector. -->
                                    <xsl:with-param name="BackList" select="$BackList"/>
                                    <xsl:with-param name="Stack" select="concat($Node,'+',$Stack)"/>
                                    <!-- Put the Node we're leaving on the stack, because
we expect to back up to it later. -->
                                </xsl:call-template>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
</xsl:stylesheet>