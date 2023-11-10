package com.sft.core.integration;

import com.google.common.base.Strings;
import com.sft.core.exception.CoreException;
import java.io.StringReader;
import java.io.StringWriter;
import java.util.HashMap;
import java.util.Map;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;

public class IntegrationTool
{
  private IntegrationTool()
  {
  }
  
  public static String integrationPackage2Xml(IntegrationPackage ipinput) throws CoreException
  {
    return integrationPackage2Xml(ipinput, false);
  }
  
  public static String integrationPackage2Xml(IntegrationPackage ipinput, boolean extended) throws CoreException
  {
    String key;
    String xml;
    Element node;
    boolean result = false;
		boolean message = false;
		boolean resultcaps = false;
		boolean messagecaps = false;

    try
    {
      DocumentBuilderFactory docFactory = DocumentBuilderFactory.newInstance();
      docFactory.setFeature("http://apache.org/xml/features/disallow-doctype-decl", true);
      DocumentBuilder docBuilder = docFactory.newDocumentBuilder();

      // root elements
      Document doc = docBuilder.newDocument();
      Element rootElement = doc.createElement("gwc");
      doc.appendChild(rootElement);

      Map map = ipinput.getInformation();

      if(map != null)
      {
        for(Object okey : map.keySet()) 
        {
          key = (String)okey;

          if(!extended)
          {
            if(key.endsWith("result"))
              result = true;
            else if(key.endsWith("RESULT"))
              resultcaps = true;
            if(key.endsWith("message"))
              message = true;
            else if(key.endsWith("MESSAGE"))
              messagecaps = true;
          }

          node = doc.createElement(key);
          node.appendChild(doc.createTextNode(Strings.nullToEmpty((String)map.get(key))));
          rootElement.appendChild(node);
        }
      }

      if(ipinput.getResult() != null)
      {
        if(!extended)
        {
          node = doc.createElement("Result");
          node.appendChild(doc.createTextNode(Strings.nullToEmpty(ipinput.getResult().toString())));
          rootElement.appendChild(node);

          if(!result)
          {
            node = doc.createElement("result");
            node.appendChild(doc.createTextNode(Strings.nullToEmpty(ipinput.getResult().toString())));
            rootElement.appendChild(node);
          }
        }

        if(!resultcaps)
        {
          node = doc.createElement("RESULT");
          node.appendChild(doc.createTextNode(Strings.nullToEmpty(ipinput.getResult().toString())));
          rootElement.appendChild(node);
        }
      }

      node = doc.createElement("Message");
      node.appendChild(doc.createTextNode(Strings.nullToEmpty(ipinput.getMessage())));
      rootElement.appendChild(node);

      if(!message && !extended)
      {
        node = doc.createElement("message");
        node.appendChild(doc.createTextNode(Strings.nullToEmpty(ipinput.getMessage())));
        rootElement.appendChild(node);
      }

      if(!messagecaps)
      {
        node = doc.createElement("MESSAGE");
        node.appendChild(doc.createTextNode(Strings.nullToEmpty(ipinput.getMessage())));
        rootElement.appendChild(node);
      }

      DOMSource domSource = new DOMSource(doc);
      StringWriter writer = new StringWriter();
      StreamResult stream = new StreamResult(writer);

      TransformerFactory tf = TransformerFactory.newInstance();
      tf.setFeature("http://xml.org/sax/features/namespaces", false);
      tf.setFeature("http://xml.org/sax/features/validation", false);
      tf.setFeature("http://apache.org/xml/features/nonvalidating/load-dtd-grammar", false);
      tf.setFeature("http://apache.org/xml/features/nonvalidating/load-external-dtd", false);

      Transformer transformer = tf.newTransformer();
      if(!extended)
      {
        transformer.setOutputProperty(OutputKeys.OMIT_XML_DECLARATION, "yes");
      }
      transformer.transform(domSource, stream);

      xml = writer.toString();

      writer.close();
    }
    catch(Exception e)
    {
      throw new CoreException(e);
    }
    
    return xml;
  }
  
  public static IntegrationPackage xml2integrationPackage(String xml)
  {
    IntegrationPackage ip = new IntegrationPackage();
    
    HashMap map = new HashMap();
    try
    {
      DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
      dbFactory.setFeature("http://apache.org/xml/features/disallow-doctype-decl", true);
      
      DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
      InputSource is = new InputSource();
      is.setCharacterStream(new StringReader(xml));
      Document doc = dBuilder.parse(is);
      
      doc.getDocumentElement().normalize();
      
      NodeList nodes = doc.getChildNodes();
      
      for (int i = 0; i < nodes.getLength(); i++)
      {
        Node currentNode = nodes.item(i);
        if (currentNode.getNodeType() == Node.ELEMENT_NODE)
        {
          map.put(currentNode.getNodeName(), currentNode.getNodeValue());
        }
      }
    }
    catch(Exception e)
    {
      // Unable to manage
    }
    
    ip.setInformation(map);
    
    return ip;
  }
  
  public static void xml2integrationPackage(String xml, IntegrationPackage ip)
  {
    HashMap map = new HashMap();
    try
    {
      DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
      dbFactory.setFeature("http://apache.org/xml/features/disallow-doctype-decl", true);
      DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
      InputSource is = new InputSource();
      is.setCharacterStream(new StringReader(xml));
      Document doc = dBuilder.parse(is);
      
      doc.getDocumentElement().normalize();
      
      NodeList nodes = doc.getFirstChild().getChildNodes();
      
      for (int i = 0; i < nodes.getLength(); i++)
      {
        Node currentNode = nodes.item(i);
        if (currentNode.getNodeType() == Node.ELEMENT_NODE)
        {
          map.put(currentNode.getNodeName(), currentNode.getFirstChild().getTextContent());
        }
      }
    }
    catch(Exception e)
    {
      // Unable to manage
    }
    
    ip.setInformation(map);
  }  
}
