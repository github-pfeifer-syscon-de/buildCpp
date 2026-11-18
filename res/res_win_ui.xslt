<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="xml" encoding="UTF-8" indent="yes"/>

  <xsl:template match="/">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="proj">
    <!-- Generated with glade 3.36.0 -->
    <interface>
      <requires lib="gtk+" version="3.22"/>
      <object class="GtkApplicationWindow" >
        <xsl:attribute name="id">
          <xsl:value-of select="@name"/>Win
        </xsl:attribute>
        <property name="can_focus">False</property>
        <child>
          <object class="GtkPaned" id="paned">
            <property name="visible">True</property>
            <property name="can_focus">True</property>
            <property name="wide_handle">True</property>
            <child>
              <object class="GtkScrolledWindow">
                <property name="visible">True</property>
                <property name="can_focus">True</property>
                <property name="shadow_type">in</property>
                <child>
                  <object class="GtkTextView" id="text_view">
                    <property name="visible">True</property>
                    <property name="can_focus">True</property>
                  </object>
                </child>
              </object>
              <packing>
                <property name="resize">False</property>
                <property name="shrink">True</property>
              </packing>
            </child>
            <child>
              <object class="GtkScrolledWindow">
                <property name="visible">True</property>
                <property name="can_focus">True</property>
                <property name="shadow_type">in</property>
                <child>
                  <object class="GtkTreeView" id="tree_view">
                    <property name="visible">True</property>
                    <property name="can_focus">True</property>
                    <child internal-child="selection">
                      <object class="GtkTreeSelection"/>
                    </child>
                  </object>
                </child>
              </object>
              <packing>
                <property name="resize">True</property>
                <property name="shrink">True</property>
              </packing>
            </child>
          </object>
        </child>
        <child type="titlebar">
          <placeholder/>
        </child>
      </object>
    </interface>

  </xsl:template>
</xsl:stylesheet>