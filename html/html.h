/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <memory>

#include <unordered_map>

#include <variant>

#include "../data.h"

#include "../db.h"

#include "utils/utils.h"

// TODO: Escape the text

namespace html {

// std::string print(const Table &t);

std::string escape(std::string input);

std::string link(std::string text, std::string url);

std::unordered_map<std::string, std::string> parse_attributes(std::string str_attr);

class Object {
  public:
    Object() {}

    ~Object() {}
};

/*
class Attribute : public Object{
private:
    std::string prv_name;
    std::string prv_value;

public:
    Attribute(std::string name, std::string value)
    {
        prv_name = name;
        prv_value = value;
    }

    ~Attribute()
    {
    }

    std::string name(){
        return prv_name;
    }

    std::string value(){
        return prv_value;
    }

    std::string str()
    {
        return prv_name + "=\"" + prv_value + "\"";
    }

};
*/

class Element : public Object {
  private:
    std::string id;
    std::string class_;

  protected:
    std::string tagName;

    std::string style;

    // TODO: Convert to smart pointer
    // std::vector< html::Attribute > attributes;

    std::unordered_map<std::string, std::string> prv_attributes;

    std::vector<html::Element *> container;

  public:
    Element() {}

    Element(std::string name) { tagName = name; }

    Element(Element *e) { insert(e); }

    ~Element() {}

    void insert(html::Element *e) { container.push_back(e); }

    void insert_attribute(std::string attribute, std::string value) { prv_attributes[attribute] = value; }

    void insert_attributes(const std::unordered_map<std::string, std::string> &attributes) {

        for (auto a : attributes) {
            insert_attribute(a.first, a.second);
        }
    }

    std::unordered_map<std::string, std::string> &attributes() { return prv_attributes; }

    std::string attribute(std::string name) {

        auto it = prv_attributes.find(name);

        if (it == prv_attributes.end()) {
            return "";
        } else {
            return it->second;
        }
    }

    virtual std::string str() {
        std::string str = "<" + tagName;

        // traverse attributes
        for (auto x : prv_attributes) {
            str += " " + x.first + "=\"" + x.second + "\"";
        }

        str += ">";

        // std::string str;

        for (auto x : container) {
            str += x->str();
        }

        str += "</" + tagName + ">";

        return str;
    }
};

class Head : public html::Element {

  public:
    Head() { tagName = "head"; }

    ~Head() {}

    void setStyle(std::string style) { this->style = style; }

    std::string str() {
        std::string str = "<" + tagName + ">";

        if (style != "") {
            str += "<style>" + style + "</style>";
        }

        // std::string str;

        for (auto x : container) {
            str += x->str();
        }

        str += "</" + tagName + ">";

        return str;
    }
};

class Body : public html::Element {
  public:
    Body() { tagName = "body"; }

    ~Body() {}
};

class root : public html::Element {
  public:
    root() {
        tagName = "html";
        insert(new html::Head());
        insert(new html::Body());
    }

    ~root() {}

    html::Head *head() { return (html::Head *)container[0]; }

    html::Body *body() { return (html::Body *)container[1]; }
};

class Table : public html::Element {
  private:
    std::vector<std::string> header;

    std::vector<std::vector<std::string>> rows;

    int numColumns = 1;

  public:
    Table(const std::initializer_list<std::string> &columns = {}) {

        if (columns.size() > 0) {

            numColumns = columns.size();

            std::vector<std::string> row;

            for (auto x : columns) {
                row.push_back(x);
            }

            rows.push_back(row);
        }
    }
    ~Table() {}

    void set_header(std::vector<std::string> header) { this->header = header; }

    void add_column(std::string column) {
        // if(rows.size() == 0){
        // rows.push_back({column});
        //}
        // else{
        rows[numColumns].push_back(column);
        //}
    }

    void add_row(std::vector<std::string> row) { rows.push_back(row); }

    std::string str() {
        std::string str = "<table";

        // traverse attributes
        for (auto x : prv_attributes) {
            str += " " + x.first + "=\"" + x.second + "\"";
        }

        str += ">";

        str += "<thead><tr>";
        for (auto x : header) {
            str += "<th>" + x + "</th>";
        }
        str += "</tr></thead>";

        str += "<tbody>";
        for (auto row : rows) {
            str += "<tr>";
            for (auto cell : row) {
                str += "<td>" + cell + "</td>";
            }
            str += "</tr>";
        }
        str += "</tbody>";

        str += "</table>";
        return str;
    }
};

class Text : public html::Element {
  private:
    std::string text;

  public:
    Text() {}

    Text(std::string text) {

        std::string tmp = text;

        this->text = html::escape(text);
    }

    Text(std::string text, const std::vector<std::pair<uint32_t, uint32_t>> &highlight) {

        // this->text = html::escape(text);

        std::string newText;

        size_t ptr = 0;

        for (int i = 0; i < highlight.size(); i++) {

            newText = html::escape(text.substr(ptr, highlight[i].first));
            newText += "<mark><b>";
            newText += html::escape(text.substr(highlight[i].first, highlight[i].second - highlight[i].first));
            newText += "</b></mark>";

            ptr = highlight[i].second;
        }

        newText += html::escape(text.substr(ptr));

        this->text = newText;
    }

    ~Text() {}

    std::string str() { return text; }
};

class Script : public html::Element {
  private:
    std::string code;

  public:
    Script() { tagName = "script"; }

    Script(std::string text) {
        tagName = "script";
        this->code = text;
    }

    ~Script() {}

    std::string str() {

        std::string str = "<" + tagName;

        // traverse attributes
        for (auto x : prv_attributes) {
            str += " " + x.first + "=\"" + x.second + "\"";
        }

        str += ">";

        // std::string str;

        str += this->code;

        str += "</" + tagName + ">";

        return str;
    }
};

class Div : public html::Element {
  public:
    Div() { tagName = "div"; }

    Div(html::Element *e) {
        tagName = "div";
        insert(e);
    }

    ~Div() {}
};

class Pre : public html::Element {
  public:
    Pre() { tagName = "pre"; }

    Pre(html::Element *e) {
        tagName = "pre";
        insert(e);
    }

    ~Pre() {}
};

class Option : public html::Element {
  public:
    Option() { tagName = "option"; }

    Option(std::string text) {
        tagName = "option";
        insert(new Text(text));
    }

    // It seems that an option tag cannot have any child elements.

    ~Option() {}
};

class Hyperlink : public html::Element {

  public:
    Hyperlink() { tagName = "a"; }

    Hyperlink(std::string href, std::string text) {

        tagName = "a";

        // html::Attribute attr("href", href);
        insert_attribute("href", href);

        insert(new Text(text));
    }

    ~Hyperlink() {}

    std::string value() { return attribute("href"); }
};

class Code : public html::Element {
  public:
    Code() { tagName = "code"; }

    Code(html::Element *e) {
        tagName = "code";
        insert(e);
    }

    ~Code() {}
};

class Dropdown : public html::Element {
  public:
    Dropdown() { tagName = "select"; }

    Dropdown(std::vector<std::string> options) {
        tagName = "select";

        for (auto o : options) {
            addOption(o);
        }
    }

    // We can only insert options in a dropdown
    Dropdown(html::Option *o) {
        tagName = "select";
        insert(o);
    }

    ~Dropdown() {}

    void addOption(std::string option) {
        Option *o = new Option(option);
        insert(o);
    }
};

// Access to Table object in Data

html::Element *convert(const data::Object *obj);

class Document {
  private:
    // html::element root{"html"};

    html::root doc;

    html::Head *head;
    html::Body *body;

  public:
    Document() {

        // html::root * ptr = ;

        // std::make_unique<html::element>("html");
        // root("pepe");

        head = doc.head();
        body = doc.body();
    }

    ~Document() {}

    void setTitle(std::string title) {
        // this->title = title;
    }

    void setStyle(std::string style) { head->setStyle(style); }

    void setBody(std::string body) {
        // this->body = body;
    }

    std::string str() {
        std::string str = "<!DOCTYPE html>\n";

        str += doc.str();

        return str;
    }

    Document &operator<<(html::Element *e) {
        body->insert(e);

        return *this;
    }

    Document &operator<<(const data::Object *obj) {

        // const data::Object &data = *d;

        html::Element *e = html::convert(obj);

        body->insert(e);

        return *this;
    }
};

} // namespace html

std::ostream &operator<<(std::ostream &stream, html::Document &doc);
