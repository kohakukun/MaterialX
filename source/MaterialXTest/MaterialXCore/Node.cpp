//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Document.h>

#include <MaterialXFormat/File.h>
#include <MaterialXFormat/XmlIo.h>
#include <MaterialXFormat/Util.h>

#include <iostream>

namespace mx = MaterialX;

bool isTopologicalOrder(const std::vector<mx::ElementPtr>& elems)
{
    std::set<mx::ElementPtr> prevElems;
    for (mx::ElementPtr elem : elems)
    {
        for (size_t i = 0; i < elem->getUpstreamEdgeCount(); i++)
        {
            mx::ElementPtr upstreamElem = elem->getUpstreamElement(i);
            if (upstreamElem && !prevElems.count(upstreamElem))
            {
                return false;
            }
        }
        prevElems.insert(elem);
    }
    return true;
}

TEST_CASE("Node", "[node]")
{
    // Create a document.
    mx::DocumentPtr doc = mx::createDocument();

    // Create a graph with two source nodes.
    mx::NodePtr constant = doc->addNode("constant");
    mx::NodePtr image = doc->addNode("image");
    REQUIRE(doc->getNodes().size() == 2);
    REQUIRE(doc->getNodes("constant").size() == 1);
    REQUIRE(doc->getNodes("image").size() == 1);

    // Set constant node color.
    mx::Color3 color(0.1f, 0.2f, 0.3f);
    constant->setInputValue<mx::Color3>("value", color);
    REQUIRE(constant->getInputValue("value")->isA<mx::Color3>());
    REQUIRE(constant->getInputValue("value")->asA<mx::Color3>() == color);

    // Set image node file.
    std::string file("image1.tif");
    image->setInputValue("file", file, mx::FILENAME_TYPE_STRING);
    REQUIRE(image->getInputValue("file")->isA<std::string>());
    REQUIRE(image->getInputValue("file")->asA<std::string>() == file);

    // Create connected outputs.
    mx::OutputPtr output1 = doc->addOutput();
    mx::OutputPtr output2 = doc->addOutput();
    output1->setConnectedNode(constant);
    output2->setConnectedNode(image);
    REQUIRE(output1->getUpstreamElement() == constant);
    REQUIRE(output2->getUpstreamElement() == image);
    REQUIRE(constant->getDownstreamPorts()[0] == output1);
    REQUIRE(image->getDownstreamPorts()[0] == output2);

    // Create a custom nodedef.
    mx::NodeDefPtr customNodeDef = doc->addNodeDef("ND_turbulence3d", "float", "turbulence3d");
    customNodeDef->setNodeGroup(mx::NodeDef::PROCEDURAL_NODE_GROUP);
    customNodeDef->setInputValue("octaves", 3);
    customNodeDef->setInputValue("lacunarity", 2.0f);
    customNodeDef->setInputValue("gain", 0.5f);

    // Reference the custom nodedef.
    mx::NodePtr custom = doc->addNodeInstance(customNodeDef);
    REQUIRE(custom->getNodeDefString() == customNodeDef->getName());
    REQUIRE(custom->getNodeDef()->getNodeGroup() == mx::NodeDef::PROCEDURAL_NODE_GROUP);
    REQUIRE(custom->getInputValue("octaves")->isA<int>());
    REQUIRE(custom->getInputValue("octaves")->asA<int>() == 3);
    custom->setInputValue("octaves", 5);
    REQUIRE(custom->getInputValue("octaves")->asA<int>() == 5);

    // Remove the nodedef attribute from the node, requiring that it fall back
    // to type and version matching.
    custom->removeAttribute(mx::NodeDef::NODE_DEF_ATTRIBUTE);
    REQUIRE(custom->getNodeDef() == customNodeDef);

    // Set nodedef and node version strings.
    customNodeDef->setVersionString("2.0");
    REQUIRE(custom->getNodeDef() == nullptr);
    customNodeDef->setDefaultVersion(true);
    REQUIRE(custom->getNodeDef() == customNodeDef);
    custom->setVersionString("1");
    REQUIRE(custom->getNodeDef() == nullptr);
    custom->removeAttribute(mx::InterfaceElement::VERSION_ATTRIBUTE);
    REQUIRE(custom->getNodeDef() == customNodeDef);

    // Define a custom type.
    mx::TypeDefPtr typeDef = doc->addTypeDef("spectrum");
    const int scalarCount = 10;
    for (int i = 0; i < scalarCount; i++)
    {
        mx::MemberPtr scalar = typeDef->addMember();
        scalar->setType("float");
    }
    REQUIRE(typeDef->getMembers().size() == scalarCount);

    // Reference the custom type.
    std::string d65("400.0,82.75,500.0,109.35,600.0,90.01,700.0,71.61,800.0,59.45");
    constant->setInputValue<std::string>("value", d65, "spectrum");
    REQUIRE(constant->getInput("value")->getType() == "spectrum");
    REQUIRE(constant->getInput("value")->getValueString() == d65);
    REQUIRE(constant->getInputValue("value")->isA<std::string>());
    REQUIRE(constant->getInputValue("value")->asA<std::string>() == d65);

    // Validate the document.
    REQUIRE(doc->validate());

    // Disconnect outputs from sources.
    output1->setConnectedNode(nullptr);
    output2->setConnectedNode(nullptr);
    REQUIRE(output1->getUpstreamElement() == nullptr);
    REQUIRE(output2->getUpstreamElement() == nullptr);
    REQUIRE(constant->getDownstreamPorts().empty());
    REQUIRE(image->getDownstreamPorts().empty());

    // Remove nodes and outputs.
    doc->removeNode(image->getName());
    doc->removeNode(constant->getName());
    doc->removeNode(custom->getName());
    doc->removeOutput(output1->getName());
    doc->removeOutput(output2->getName());
    REQUIRE(doc->getNodes().empty());
    REQUIRE(doc->getOutputs().empty());
}

TEST_CASE("Flatten", "[nodegraph]")
{
    mx::FileSearchPath searchPath = "resources/Materials/Examples/Syntax" +
                                    mx::PATH_LIST_SEPARATOR +
                                    "libraries/stdlib";

    // Read the example file.
    mx::DocumentPtr doc = mx::createDocument();
    mx::readFromXmlFile(doc, "SubGraphs.mtlx", searchPath);

    // Find the example graph.
    mx::NodeGraphPtr graph = doc->getNodeGraph("subgraph_ex1");
    REQUIRE(graph);

    // Traverse the graph and count nodes.
    int totalNodeCount = 0;
    for (mx::ElementPtr elem : graph->traverseTree())
    {
        if (elem->isA<mx::Node>())
        {
            totalNodeCount++;
        }
    }
    REQUIRE(totalNodeCount == 7);

    // Create a flat version of the graph.
    mx::NodeGraphPtr flatGraph = doc->addNodeGraph();
    flatGraph->copyContentFrom(graph);
    flatGraph->flattenSubgraphs();

    // Traverse the flat graph and count nodes.
    totalNodeCount = 0;
    for (mx::ElementPtr elem : flatGraph->traverseTree())
    {
        if (elem->isA<mx::Node>())
        {
            totalNodeCount++;

            // Make sure it's an atomic node.
            mx::InterfaceElementPtr implement = elem->asA<mx::Node>()->getImplementation();
            bool isAtomic = !implement || !implement->isA<mx::NodeGraph>();
            REQUIRE(isAtomic);
        }
    }
    REQUIRE(totalNodeCount == 15);

    // Test filtered flattening. Leave one node unflattened
    flatGraph->copyContentFrom(graph);
    flatGraph->flattenSubgraphs(mx::EMPTY_STRING, [] (mx::NodePtr node)
    {
        return (node->getCategory() != "color_checker");
    });
    totalNodeCount = 0;
    for (mx::ElementPtr elem : flatGraph->traverseTree())
    {
        if (elem->isA<mx::Node>())
        {
            totalNodeCount++;

            // Make sure it's an atomic node.
            mx::InterfaceElementPtr implement = elem->asA<mx::Node>()->getImplementation();
            bool isAtomic = !implement || !implement->isA<mx::NodeGraph>();
            if (elem->getCategory() != "color_checker")
            {
                REQUIRE(isAtomic);
            }
        }
    }
    REQUIRE(totalNodeCount == 16);
}

TEST_CASE("Topological sort", "[nodegraph]")
{
    // Create a document.
    mx::DocumentPtr doc = mx::createDocument();

    // Create a node graph with the following structure:
    //
    //   [constant1] [constant2]      [image2]
    //           \   /          \    /
    // [image1] [add1]          [add2]
    //        \  /   \______      |   
    //    [multiply]        \__ [add3]         [noise3d]
    //             \____________  |  ____________/
    //                          [mix]
    //                            |
    //                         [output]
    //
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph();
    mx::NodePtr image1 = nodeGraph->addNode("image");
    mx::NodePtr image2 = nodeGraph->addNode("image");
    mx::NodePtr multiply = nodeGraph->addNode("multiply");
    mx::NodePtr constant1 = nodeGraph->addNode("constant");
    mx::NodePtr constant2 = nodeGraph->addNode("constant");
    mx::NodePtr add1 = nodeGraph->addNode("add");
    mx::NodePtr add2 = nodeGraph->addNode("add");
    mx::NodePtr add3 = nodeGraph->addNode("add");
    mx::NodePtr noise3d = nodeGraph->addNode("noise3d");
    mx::NodePtr mix = nodeGraph->addNode("mix");
    mx::OutputPtr output = nodeGraph->addOutput();
    add1->setConnectedNode("in1", constant1);
    add1->setConnectedNode("in2", constant2);
    add2->setConnectedNode("in1", constant2);
    add2->setConnectedNode("in2", image2);
    add3->setConnectedNode("in1", add1);
    add3->setConnectedNode("in2", add2);
    multiply->setConnectedNode("in1", image1);
    multiply->setConnectedNode("in2", add1);
    mix->setConnectedNode("fg", multiply);
    mix->setConnectedNode("bg", add3);
    mix->setConnectedNode("mask", noise3d);
    output->setConnectedNode(mix);

    // Validate the document.
    REQUIRE(doc->validate());

    // Create a topological order and validate the results.
    std::vector<mx::ElementPtr> elemOrder = nodeGraph->topologicalSort();
    REQUIRE(elemOrder.size() == nodeGraph->getChildren().size());
    REQUIRE(isTopologicalOrder(elemOrder));
}

TEST_CASE("New nodegraph from output", "[nodegraph]")
{
    // Create a document.
    mx::DocumentPtr doc = mx::createDocument();

    // Create a node graph with the following structure:
    //
    //   [constant1] [constant2]      [image2]
    //           \   /          \    /
    // [image1] [add1]          [add2]
    //        \  /   \______      |   
    //   [multiply1]        \__ [add3]         [noise3d]            [constant3]
    //             \____________  |  ____________/    \                /
    //                          [mix]                  \_ [multiply2]_/
    //                            |                           |
    //                          [out1]                      [out2]
    //
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph();
    mx::NodePtr image1 = nodeGraph->addNode("image");
    mx::NodePtr image2 = nodeGraph->addNode("image");
    mx::NodePtr multiply1 = nodeGraph->addNode("multiply");
    mx::NodePtr multiply2 = nodeGraph->addNode("multiply");
    mx::NodePtr constant1 = nodeGraph->addNode("constant");
    mx::NodePtr constant2 = nodeGraph->addNode("constant");
    mx::NodePtr constant3 = nodeGraph->addNode("constant");
    mx::NodePtr add1 = nodeGraph->addNode("add");
    mx::NodePtr add2 = nodeGraph->addNode("add");
    mx::NodePtr add3 = nodeGraph->addNode("add");
    mx::NodePtr noise3d = nodeGraph->addNode("noise3d");
    mx::NodePtr mix = nodeGraph->addNode("mix");
    mx::OutputPtr out1 = nodeGraph->addOutput("out1");
    mx::OutputPtr out2 = nodeGraph->addOutput("out2");
    add1->setConnectedNode("in1", constant1);
    add1->setConnectedNode("in2", constant2);
    add2->setConnectedNode("in1", constant2);
    add2->setConnectedNode("in2", image2);
    add3->setConnectedNode("in1", add1);
    add3->setConnectedNode("in2", add2);
    multiply1->setConnectedNode("in1", image1);
    multiply1->setConnectedNode("in2", add1);
    multiply2->setConnectedNode("in1", noise3d);
    multiply2->setConnectedNode("in2", constant3);
    mix->setConnectedNode("fg", multiply1);
    mix->setConnectedNode("bg", add3);
    mix->setConnectedNode("mask", noise3d);
    out1->setConnectedNode(mix);
    out2->setConnectedNode(multiply2);

    // Generate a new graph from each output.
    std::vector<mx::OutputPtr> outputs = {out1, out2};
    for (size_t i = 0; i < outputs.size(); ++i)
    {
        const mx::OutputPtr output = outputs[i];

        // Create a new graph with this output.
        mx::NodeGraphPtr nodeGraph2 = doc->addNodeGraph();
        nodeGraph2->addOutput(output->getName());

        // Keep track of processed nodes to avoid duplication
        // of nodes with multiple downstream connections.
        std::set<mx::NodePtr> processedNodes;

        for (mx::Edge edge : output->traverseGraph())
        {
            mx::NodePtr upstreamNode = edge.getUpstreamElement()->asA<mx::Node>();
            if (processedNodes.count(upstreamNode))
            {
                // Node is already processed
                continue;
            }

            // Create this node in the new graph.
            mx::NodePtr newNode = nodeGraph2->addNode(upstreamNode->getCategory(), upstreamNode->getName());
            newNode->copyContentFrom(upstreamNode);

            // Connect the node to downstream element in the new graph.
            mx::ElementPtr downstreamElement = edge.getDownstreamElement();
            mx::ElementPtr connectingElement = edge.getConnectingElement();
            if (downstreamElement->isA<mx::Output>())
            {
                mx::OutputPtr downstream = nodeGraph2->getOutput(downstreamElement->getName());
                downstream->setConnectedNode(newNode);
            }
            else if (connectingElement)
            {
                mx::NodePtr downstream = nodeGraph2->getNode(downstreamElement->getName());
                downstream->setConnectedNode(connectingElement->getName(), newNode);
            }

            // Mark node as processed.
            processedNodes.insert(upstreamNode);
        }

        // Create a topological order and validate the results.
        std::vector<mx::ElementPtr> elemOrder = nodeGraph2->topologicalSort();
        REQUIRE(elemOrder.size() == nodeGraph2->getChildren().size());
        REQUIRE(isTopologicalOrder(elemOrder));
    }

    // Validate the document.
    REQUIRE(doc->validate());
}

TEST_CASE("Prune nodes", "[nodegraph]")
{
    // Create a document.
    mx::DocumentPtr doc = mx::createDocument();

    // Create a node graph with the following structure:
    //
    //   [constant1] [constant2]      [image2]
    //           \   /          \    /
    // [image1] [add1]          [add2]
    //        \  /   \______      |   
    //    [multiply]        \__ [add3]         [noise3d]
    //             \____________  |  ____________/
    //                          [mix]
    //                            |
    //                         [output]
    //
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph();
    mx::NodePtr image1 = nodeGraph->addNode("image");
    mx::NodePtr image2 = nodeGraph->addNode("image");
    mx::NodePtr multiply = nodeGraph->addNode("multiply");
    mx::NodePtr constant1 = nodeGraph->addNode("constant");
    mx::NodePtr constant2 = nodeGraph->addNode("constant");
    mx::NodePtr add1 = nodeGraph->addNode("add");
    mx::NodePtr add2 = nodeGraph->addNode("add");
    mx::NodePtr add3 = nodeGraph->addNode("add");
    mx::NodePtr noise3d = nodeGraph->addNode("noise3d");
    mx::NodePtr mix = nodeGraph->addNode("mix");
    mx::OutputPtr output = nodeGraph->addOutput();
    add1->setConnectedNode("in1", constant1);
    add1->setConnectedNode("in2", constant2);
    add2->setConnectedNode("in1", constant2);
    add2->setConnectedNode("in2", image2);
    add3->setConnectedNode("in1", add1);
    add3->setConnectedNode("in2", add2);
    multiply->setConnectedNode("in1", image1);
    multiply->setConnectedNode("in2", add1);
    mix->setConnectedNode("fg", multiply);
    mix->setConnectedNode("bg", add3);
    mix->setConnectedNode("mask", noise3d);
    output->setConnectedNode(mix);

    // Set the node names we want to prune from the graph
    // and which corresponding input to use for the bypass.
    std::unordered_map<std::string, std::string> nodesToPrune =
    {
        { "add1","in1" },
        { "add2","in1" },
        { "add3","in1" }
    };

    // Keep track of processed nodes to avoid duplication
    // of nodes with multiple downstream connections.
    std::set<mx::NodePtr> processedNodes;

    // Create the new graph with this output and traverse the
    // original graph upstream to find which nodes to copy.
    mx::NodeGraphPtr nodeGraph2 = doc->addNodeGraph();
    nodeGraph2->addOutput(output->getName());

    for (mx::Edge edge : output->traverseGraph())
    {
        mx::NodePtr upstreamNode = edge.getUpstreamElement()->asA<mx::Node>();
        if (processedNodes.count(upstreamNode))
        {
            // Node is already processed.
            continue;
        }

        // Find the downstream element in the new graph.
        mx::ElementPtr downstreamElement = edge.getDownstreamElement();
        mx::ElementPtr downstreamElement2 = nodeGraph2->getChild(downstreamElement->getName());
        if (!downstreamElement2)
        {
            // Downstream element has been pruned
            // so ignore this edge.
            continue;
        }

        // Check if this node should be pruned.
        // If so we travers upstream using the bypass inputs
        // until a non-prune node is found.
        mx::ValuePtr value;
        while (upstreamNode)
        {
            if (!nodesToPrune.count(upstreamNode->getName()))
            {
                break;
            }
            const std::string& inputName = nodesToPrune[upstreamNode->getName()];
            upstreamNode = upstreamNode->getConnectedNode(inputName);
        }

        if (upstreamNode)
        {
            // Get (or create) the node in the new graph.
            mx::NodePtr upstreamNode2 = nodeGraph2->getNode(upstreamNode->getName());
            if (!upstreamNode2)
            {
                upstreamNode2 = nodeGraph2->addNode(upstreamNode->getCategory(), upstreamNode->getName());
                upstreamNode2->copyContentFrom(upstreamNode);
            }

            mx::ElementPtr connectingElement = edge.getConnectingElement();

            // Connect it to downstream.
            // The downstream element could be a node or an output.
            mx::NodePtr downstreamNode2 = downstreamElement2->asA<mx::Node>();
            mx::OutputPtr downstreamOutput2 = downstreamElement2->asA<mx::Output>();
            if (downstreamOutput2)
            {
                downstreamOutput2->setConnectedNode(upstreamNode2);
            }
            else if (downstreamNode2 && connectingElement)
            {
                downstreamNode2->setConnectedNode(connectingElement->getName(), upstreamNode2);
            }
        }

        // Mark node as processed.
        processedNodes.insert(upstreamNode);
    }

    // Validate the document.
    REQUIRE(doc->validate());

    // Create a topological order and validate the results.
    std::vector<mx::ElementPtr> elemOrder = nodeGraph2->topologicalSort();
    REQUIRE(elemOrder.size() == nodeGraph2->getChildren().size());
    REQUIRE(isTopologicalOrder(elemOrder));
}

TEST_CASE("Organization", "[nodegraph]")
{
    // Create a document.
    mx::DocumentPtr doc = mx::createDocument();

    // Create a node graph with the following structure:
    //
    //   [constant1] [constant2]      [image2]
    //           \   /          \    /
    // [image1] [add1]          [add2]
    //        \  /   \______      |   
    //    [multiply]        \__ [add3]         [noise3d]
    //             \____________  |  ____________/
    //                          [mix]
    //                            |
    //                         [output]
    //
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph();
    mx::NodePtr image1 = nodeGraph->addNode("image");
    mx::NodePtr image2 = nodeGraph->addNode("image");
    mx::NodePtr multiply = nodeGraph->addNode("multiply");
    mx::NodePtr constant1 = nodeGraph->addNode("constant");
    mx::NodePtr constant2 = nodeGraph->addNode("constant");
    mx::NodePtr add1 = nodeGraph->addNode("add");
    mx::NodePtr add2 = nodeGraph->addNode("add");
    mx::NodePtr add3 = nodeGraph->addNode("add");
    mx::NodePtr noise3d = nodeGraph->addNode("noise3d");
    mx::NodePtr mix = nodeGraph->addNode("mix");
    mx::OutputPtr output = nodeGraph->addOutput();
    add1->setConnectedNode("in1", constant1);
    add1->setConnectedNode("in2", constant2);
    add2->setConnectedNode("in1", constant2);
    add2->setConnectedNode("in2", image2);
    add3->setConnectedNode("in1", add1);
    add3->setConnectedNode("in2", add2);
    multiply->setConnectedNode("in1", image1);
    multiply->setConnectedNode("in2", add1);
    mix->setConnectedNode("fg", multiply);
    mix->setConnectedNode("bg", add3);
    mix->setConnectedNode("mask", noise3d);
    output->setConnectedNode(mix);

    // Create a backdrop element.
    mx::BackdropPtr backdrop1 = nodeGraph->addBackdrop();
    backdrop1->setContainsElements({ constant1, constant2, add1 });
    backdrop1->setDocString("Group 1");
    backdrop1->setWidth(10.0f);
    backdrop1->setHeight(20.0f);
    CHECK(backdrop1->getContainsElements().size() == 3);
    CHECK(backdrop1->getContainsElements()[0] == constant1);
    CHECK(backdrop1->getDocString() == "Group 1");
    CHECK(backdrop1->getWidth() == 10.0f);
    CHECK(backdrop1->getHeight() == 20.0f);

    // Create a second backdrop element.
    mx::BackdropPtr backdrop2 = nodeGraph->addBackdrop();
    backdrop2->setContainsElements({ multiply, noise3d, mix, output });
    backdrop2->setDocString("Group 2");
    backdrop2->setWidth(30.0f);
    backdrop2->setHeight(40.0f);
    CHECK(backdrop2->getContainsElements().size() == 4);
    CHECK(backdrop2->getContainsElements()[0] == multiply);
    CHECK(backdrop2->getDocString() == "Group 2");
    CHECK(backdrop2->getWidth() == 30.0f);
    CHECK(backdrop2->getHeight() == 40.0f);

    // Validate the document.
    REQUIRE(doc->validate());

    // Create and test an invalid contains element.
    backdrop2->setContainsElements({ nodeGraph });
    REQUIRE(!doc->validate());

    // Remove backdrops.
    nodeGraph->removeBackdrop(backdrop1->getName());
    nodeGraph->removeBackdrop(backdrop2->getName());
    CHECK(nodeGraph->getBackdrops().empty());
}

TEST_CASE("Tokens", "[nodegraph]")
{
    mx::DocumentPtr doc = mx::createDocument();
    mx::loadLibrary(mx::FilePath::getCurrentPath() / mx::FilePath("libraries/stdlib/stdlib_defs.mtlx"), doc);
    mx::loadLibrary(mx::FilePath::getCurrentPath() / mx::FilePath("libraries/stdlib/stdlib_ng.mtlx"), doc);
    mx::FileSearchPath searchPath("resources/Materials/TestSuite/stdlib/texture/");

    mx::readFromXmlFile(doc, "tokenGraph.mtlx", searchPath);
    doc->validate();

    mx::StringVec graphNames = { "Tokenized_Image_2k_png", "Tokenized_Image_4k_jpg" };
    mx::StringVec resolutionStrings = { "2k", "4k" };
    mx::StringVec extensionStrings = { "png", "jpg" };
    for (size_t i=0; i<graphNames.size(); i++)
    {
        mx::NodeGraphPtr graph = doc->getNodeGraph(graphNames[i]);
        REQUIRE(graph);
        std::vector<mx::TokenPtr> tokens = graph->getActiveTokens();

        mx::NodePtr imagePtr = graph->getNode("tiledimage");
        REQUIRE(imagePtr);

        mx::InputPtr input = imagePtr->getInput("file");
        REQUIRE(input);

        // Test file name substitution creation.
        mx::StringResolverPtr resolver = input->createStringResolver();
        const mx::StringMap& substitutions = resolver->getFilenameSubstitutions();
        const std::string DELIMITER_PREFIX("[");
        const std::string DELIMITER_POSTFIX("]");
        for (auto token : tokens)
        {
            const std::string tokenString = DELIMITER_PREFIX + token->getName() + DELIMITER_POSTFIX;
            REQUIRE(substitutions.count(tokenString));
            REQUIRE(substitutions.find(tokenString)->second == token->getValueString());
        }

        // Test that one of the tokens was used
        REQUIRE(input->getValueString() == std::string("resources/images/cloth.[Image_Extension]"));
        REQUIRE(input->getResolvedValueString() == std::string("resources/images/cloth." + extensionStrings[i]));

        // Modify and test that both of the tokens was used
        input->setValueString("resources/images/cloth_[Image_Resolution].[Image_Extension]");
        REQUIRE(input->getResolvedValueString() == std::string("resources/images/cloth_" + resolutionStrings[i] + "." + extensionStrings[i]));

        // Modify and test without proper delimiters
        input->setValueString("resources/images/cloth_<Image_Resolution>.<Image_Extension>");
        REQUIRE(input->getResolvedValueString() == std::string("resources/images/cloth_<Image_Resolution>.<Image_Extension>"));
    }
}

TEST_CASE("Node Definition Creation", "[nodedef]")
{
    mx::DocumentPtr doc = mx::createDocument();
    mx::loadLibrary(mx::FilePath::getCurrentPath() / mx::FilePath("libraries/stdlib/stdlib_defs.mtlx"), doc);
    mx::loadLibrary(mx::FilePath::getCurrentPath() / mx::FilePath("libraries/stdlib/stdlib_ng.mtlx"), doc);
    mx::FileSearchPath searchPath("resources/Materials/TestSuite/stdlib/definition/");

    mx::readFromXmlFile(doc, "definition_from_nodegraph.mtlx", searchPath);
    REQUIRE(doc->validate());

    mx::NodeGraphPtr graph = doc->getNodeGraph("test_colorcorrect");
    REQUIRE(graph);
    if (graph)
    {
        const std::string VERSION1 = "1.0";
        const std::string NAMESPACE = "namespace1";
        const std::string GROUP = "adjustment";
        bool isDefaultVersion = false;
        const std::string NODENAME = graph->getName();

        // Duplicate the graph and then make the duplicate a nodedef nodegraph
        std::string newNodeDefName = doc->createValidChildName("ND_" + graph->getName());
        std::string newGraphName = doc->createValidChildName("NG_" + graph->getName());
        mx::NodeDefPtr nodeDef = doc->addNodeDefFromGraph(graph, newNodeDefName, NODENAME, VERSION1, isDefaultVersion, 
                                                          GROUP, newGraphName, NAMESPACE);
        REQUIRE(nodeDef != nullptr);
        REQUIRE(nodeDef->getNodeGroup() == "adjustment");
        REQUIRE(nodeDef->getVersionString() == VERSION1);
        REQUIRE(nodeDef->getNamespace() == NAMESPACE);
        REQUIRE_FALSE(nodeDef->getDefaultVersion());

        // Try and fail to create the same definition
        mx::NodeDefPtr temp;
        try
        {
            temp = nullptr;
            temp = doc->addNodeDefFromGraph(graph, newNodeDefName, NODENAME, VERSION1, isDefaultVersion, GROUP, newGraphName, NAMESPACE);
        }
        catch (mx::Exception&)
        {
            REQUIRE(temp == nullptr);
        }

        // Check that the new nodegraph has the correct definition
        mx::NodeGraphPtr newGraph = doc->getNodeGraph(newGraphName);
        REQUIRE(newGraph != nullptr);
        REQUIRE(newGraph->getNodeDefString() == NAMESPACE + ":" + newNodeDefName);
        REQUIRE(newGraph->getNamespace() == NAMESPACE);

        mx::writeToXmlFile(doc, "definition_from_nodegraph_out1.mtlx");

        // Check declaration was set up properly
        mx::ConstNodeDefPtr decl = newGraph->getNodeDef();
        REQUIRE((decl && decl->getName() == nodeDef->getName()));

        // Arbitrarily add all unconnected inputs as interfaces
        mx::ValueElementPtr newInterface = nullptr;
        for (auto node : newGraph->getNodes())
        {
            mx::NodeDefPtr nodeNodeDef = node->getNodeDef();
            REQUIRE(nodeNodeDef);
            for (auto nodeDefValueElem : nodeNodeDef->getActiveValueElements())
            {
                const std::string& valueElemName = nodeDefValueElem->getName();
                mx::ValueElementPtr valueElem = node->getValueElement(valueElemName);
                if (!valueElem)
                {
                    valueElem = node->addInputFromNodeDef(valueElemName);
                    if (!valueElem)
                    {
                        continue;
                    }
                }

                mx::InputPtr input = valueElem->asA<mx::Input>();
                if (input && !input->getConnectedNode())
                {
                    std::string interfaceName = input->getNamePath();
                    interfaceName = nodeDef->createValidChildName(interfaceName);
                    newGraph->addInterfaceName(input->getNamePath(newGraph), interfaceName);
                    REQUIRE(nodeDef->getChild(interfaceName));
                    try
                    {
                        // Check duplicate failure case
                        newGraph->addInterfaceName(input->getNamePath(newGraph), interfaceName);
                    }
                    catch (mx::Exception& e)
                    {
                        REQUIRE(e.what());
                        newGraph->removeInterfaceName(input->getNamePath(newGraph));
                        REQUIRE(nodeDef->getChild(interfaceName) == nullptr);
                        newGraph->addInterfaceName(input->getNamePath(newGraph), interfaceName);

                        const std::string newInterfaceName = interfaceName + "_renamed";
                        newGraph->modifyInterfaceName(input->getNamePath(newGraph), newInterfaceName);
                        REQUIRE(nodeDef->getChild(newInterfaceName));
                    }
                }
            }
        }

        // Add new version 
        const std::string VERSION2 = "2.0";
        newGraphName = mx::EMPTY_STRING;
        nodeDef = doc->addNodeDefFromGraph(graph, newNodeDefName + "2", NODENAME, VERSION2, isDefaultVersion, GROUP, newGraphName);
        nodeDef->setDefaultVersion(true);
        REQUIRE(nodeDef != nullptr);

        std::vector<mx::NodeDefPtr> matchingNodeDefs;
        for (auto docNodeDef : doc->getNodeDefs())
        {
            if (docNodeDef->getNodeString() == NODENAME)
            {
                matchingNodeDefs.push_back(docNodeDef);
            }
        }
        bool findDefault = false;
        for (auto matchingDef : matchingNodeDefs)
        {
            if (matchingDef->getDefaultVersion())
            {
                findDefault = true;
                REQUIRE(matchingDef->getVersionString() == VERSION2);
                break;
            }
        }
        REQUIRE(findDefault);
    }

    REQUIRE(doc->validate());
    mx::writeToXmlFile(doc, "definition_from_nodegraph_out2.mtlx");
}
