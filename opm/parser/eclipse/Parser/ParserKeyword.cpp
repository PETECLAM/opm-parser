/*
  Copyright 2013 Statoil ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string>
#include <stdexcept>



#include <opm/json/JsonObject.hpp>

#include <opm/parser/eclipse/Parser/ParserConst.hpp>
#include <opm/parser/eclipse/Parser/ParserKeyword.hpp>
#include <opm/parser/eclipse/Parser/ParserIntItem.hpp>
#include <opm/parser/eclipse/Parser/ParserDoubleItem.hpp>
#include <opm/parser/eclipse/Parser/ParserStringItem.hpp>


namespace Opm {

    void ParserKeyword::commonInit(const std::string& name , ParserKeywordActionEnum action) {
        if (!validName(name))
            throw std::invalid_argument("Invalid name: " + name + "keyword must be all upper case, max 8 characters. Starting with character.");
        
        m_keywordSizeType = SLASH_TERMINATED;
        m_isDataKeyword = false;
        m_isTableCollection = false;
        m_name = name;
        m_action = action;
        m_record = ParserRecordPtr(new ParserRecord);
    }


    ParserKeyword::ParserKeyword(const std::string& name, ParserKeywordActionEnum action) {
        commonInit(name , action);
        m_action = action;
    }


    ParserKeyword::ParserKeyword(const char * name , ParserKeywordActionEnum action) {
        commonInit(name , action);
        m_action = action;
    }


    ParserKeyword::ParserKeyword(const std::string& name, size_t fixedKeywordSize  , ParserKeywordActionEnum action) {
        commonInit(name,action);
        m_keywordSizeType = FIXED;
        m_fixedSize = fixedKeywordSize;
    }


    ParserKeyword::ParserKeyword(const std::string& name ,  const std::string& sizeKeyword , const std::string& sizeItem , ParserKeywordActionEnum action , bool isTableCollection) {
        commonInit(name,action);
        m_isTableCollection = isTableCollection;
        initSizeKeyword(sizeKeyword , sizeItem);
    }

    

    bool ParserKeyword::isTableCollection() const {
        return m_isTableCollection;
    }

    void ParserKeyword::initSize( const Json::JsonObject& jsonConfig ) {
        // The number of record has been set explicitly with the size: keyword
        if (jsonConfig.has_item("size")) {
            Json::JsonObject sizeObject = jsonConfig.get_item("size");
            
            if (sizeObject.is_number()) {
                m_fixedSize = (size_t) sizeObject.as_int( );
                m_keywordSizeType = FIXED;
            } else
                initSizeKeyword( sizeObject );

        } else
            if (jsonConfig.has_item("num_tables")) {
             Json::JsonObject numTablesObject = jsonConfig.get_item("num_tables");

             if (!numTablesObject.is_object())
                 throw std::invalid_argument("The num_tables key must point to a {} object");

             initSizeKeyword( numTablesObject );
             m_isTableCollection = true;
        } else {
            if (jsonConfig.has_item("items"))
                // The number of records is undetermined - the keyword will be '/' terminated.
                m_keywordSizeType = SLASH_TERMINATED;
            else {
                m_keywordSizeType = FIXED;
                if (jsonConfig.has_item("data"))
                    m_fixedSize = 1;
                else
                    m_fixedSize = 0;
            }
        }
    }


    
    ParserKeyword::ParserKeyword(const Json::JsonObject& jsonConfig) {
        ParserKeywordActionEnum action = INTERNALIZE;

        if (jsonConfig.has_item("action"))
            action = ParserKeywordActionEnumFromString( jsonConfig.get_string("action") );
        
        if (jsonConfig.has_item("name")) {
            commonInit(jsonConfig.get_string("name") , action);
        } else
            throw std::invalid_argument("Json object is missing name: property");

        initSize( jsonConfig );

        if (jsonConfig.has_item("items")) 
            addItems( jsonConfig );
        
        if (jsonConfig.has_item("data"))
            initData( jsonConfig );

        if (isTableCollection())
            addTableItems();

        if ((m_fixedSize == 0 && m_keywordSizeType == FIXED) || (m_action != INTERNALIZE))
            return;
        else  {
            if (numItems() == 0)
                throw std::invalid_argument("Json object for keyword: " + jsonConfig.get_string("name") + " is missing items specifier");
        }
    }

    



    void ParserKeyword::initSizeKeyword( const std::string& sizeKeyword, const std::string& sizeItem) {
        m_keywordSizeType = OTHER;
        m_sizeDefinitionPair = std::pair<std::string , std::string>(sizeKeyword , sizeItem);
    }

    
    void ParserKeyword::initSizeKeyword(const Json::JsonObject& sizeObject) {
        std::string sizeKeyword = sizeObject.get_string("keyword");
        std::string sizeItem = sizeObject.get_string("item");
        initSizeKeyword( sizeKeyword , sizeItem);
    }


    bool ParserKeyword::validName(const std::string& name) {
        if (name.length() > ParserConst::maxKeywordLength)
            return false;

        if (!isupper(name[0]))
            return false;
        
        for (unsigned int i = 1; i < name.length(); i++) {
            char c = name[i];
            if ( !(isupper(c) || isdigit(c)) )
                return false;
        }
        return true;
    }





    void ParserKeyword::addItem( ParserItemConstPtr item ) {
        if (m_isDataKeyword)
            throw std::invalid_argument("Keyword:" + getName() + " is already configured as data keyword - can not add more items.");
        
        m_record->addItem( item );
    }

    
    void ParserKeyword::addDataItem( ParserItemConstPtr item ) {
        if (m_record->size())
            throw std::invalid_argument("Keyword:" + getName() + " already has items - can not add a data item.");
        
        if ((m_fixedSize == 1U) && (m_keywordSizeType == FIXED)) {
            addItem( item );
            m_isDataKeyword = true;
        } else
            throw std::invalid_argument("Keyword:" + getName() + ". When calling addDataItem() the keyword must be configured with fixed size == 1.");
    }


    void ParserKeyword::addItems( const Json::JsonObject& jsonConfig) {
        const Json::JsonObject itemsConfig = jsonConfig.get_item("items");
        if (itemsConfig.is_array()) {
            size_t num_items = itemsConfig.size();
            for (size_t i=0; i < num_items; i++) {
                const Json::JsonObject itemConfig = itemsConfig.get_array_item( i );
                
                if (itemConfig.has_item("value_type")) {
                    ParserValueTypeEnum valueType = ParserValueTypeEnumFromString( itemConfig.get_string("value_type") );
                    switch( valueType ) {
                    case INT:
                        {
                            ParserIntItemConstPtr item = ParserIntItemConstPtr(new ParserIntItem( itemConfig ));
                            addItem( item );
                        }
                        break;
                    case STRING:
                        {
                            ParserStringItemConstPtr item = ParserStringItemConstPtr(new ParserStringItem( itemConfig ));
                            addItem( item );
                        }
                        break;
                    case FLOAT:
                        {
                            ParserDoubleItemConstPtr item = ParserDoubleItemConstPtr(new ParserDoubleItem( itemConfig ));
                            addItem( item );
                        }
                        break;
                    default:
                        throw std::invalid_argument("Not implemented.");
                    }
                } else
                    throw std::invalid_argument("Json config object missing \"value_type\": ... item");            
            }
        } else
            throw std::invalid_argument("The items: object must be an array");            
    }


    void ParserKeyword::addTableItems() {
        ParserDoubleItemConstPtr item = ParserDoubleItemConstPtr(new ParserDoubleItem("TABLEROW" , ALL , 0));
        addItem( item );
    }


    void ParserKeyword::initData(const Json::JsonObject& jsonConfig) {
        m_fixedSize = 1U;
        m_keywordSizeType = FIXED;

        const Json::JsonObject dataConfig = jsonConfig.get_item("data");
        if (dataConfig.has_item("value_type")) {
            ParserValueTypeEnum valueType = ParserValueTypeEnumFromString( dataConfig.get_string("value_type") );
            const std::string itemName( getName() );
            bool hasDefault = dataConfig.has_item("default");
            
            switch( valueType ) {
                case INT:
                {
                    ParserIntItemPtr item = ParserIntItemPtr(new ParserIntItem( itemName , ALL ));
                    if (hasDefault) {
                        int defaultValue = dataConfig.get_int("default");
                        item->setDefault( defaultValue );
                    }
                    addDataItem( item );
                }
                break;
            case STRING:
                {
                    ParserStringItemPtr item = ParserStringItemPtr(new ParserStringItem( itemName , ALL ));
                    if (hasDefault) {
                        std::string defaultValue = dataConfig.get_string("default");
                        item->setDefault( defaultValue );
                    }
                    addDataItem( item );
                }
                break;
            case FLOAT:
                {
                    ParserDoubleItemPtr item = ParserDoubleItemPtr(new ParserDoubleItem( itemName , ALL ));
                    if (hasDefault) {
                        double defaultValue = dataConfig.get_double("default");
                        item->setDefault( defaultValue );
                    }
                    addDataItem( item );
                }
                break;
            default:
                throw std::invalid_argument("Not implemented.");
            } 
        } else
            throw std::invalid_argument("Json config object missing \"value_type\": ... item");            
    }


    ParserRecordPtr ParserKeyword::getRecord() const {
        return m_record;
    }

  
    ParserKeywordActionEnum ParserKeyword::getAction() const {
        return m_action;
    }


    const std::string& ParserKeyword::getName() const {
        return m_name;
    }

    size_t ParserKeyword::numItems() const {
        return m_record->size();
    }


    DeckKeywordPtr ParserKeyword::parse(RawKeywordConstPtr rawKeyword) const {
      DeckKeywordPtr keyword(new DeckKeyword(getName()));
        for (size_t i = 0; i < rawKeyword->size(); i++) {
            DeckRecordConstPtr deckRecord = m_record->parse(rawKeyword->getRecord(i));
            keyword->addRecord(deckRecord);
        }
        
        return keyword;
    }

    size_t ParserKeyword::getFixedSize() const {
        if (!hasFixedSize()) {
            throw std::logic_error("This parser keyword does not have a fixed size!");
        }
        return m_fixedSize;
    }

    bool ParserKeyword::hasFixedSize() const {
        return m_keywordSizeType == FIXED;
    }

    enum ParserKeywordSizeEnum ParserKeyword::getSizeType() const {
        return m_keywordSizeType;
    }

    const std::pair<std::string,std::string>& ParserKeyword::getSizeDefinitionPair() const {
        return m_sizeDefinitionPair;
    }

    bool ParserKeyword::isDataKeyword() const {
        return m_isDataKeyword;
    }

    bool ParserKeyword::equal(const ParserKeyword& other) const {
        if ((m_name == other.m_name) &&
            (m_record->equal( *(other.m_record) )) &&
            (m_keywordSizeType == other.m_keywordSizeType) &&
            (m_isDataKeyword == other.m_isDataKeyword) &&
            (m_isTableCollection == other.m_isTableCollection) &&
            (m_action == other.m_action)) 
            {
                bool equal = false;
                switch(m_keywordSizeType) {
                case FIXED:
                    if (m_fixedSize == other.m_fixedSize)
                        equal = true;
                    break;
                case SLASH_TERMINATED:
                    equal = true;
                    break;
                case OTHER:
                    if ((m_sizeDefinitionPair.first  == other.m_sizeDefinitionPair.first) &&
                        (m_sizeDefinitionPair.second == other.m_sizeDefinitionPair.second))
                        equal = true;
                    break;
                }
                return equal;
            }
        else
            return false;
    }


  void ParserKeyword::inlineNew(std::ostream& os , const std::string& lhs, const std::string& indent) const {
      {
          const std::string actionString(ParserKeywordActionEnum2String( m_action ));
          switch(m_keywordSizeType) {
          case SLASH_TERMINATED:
              os << lhs << " = new ParserKeyword(\"" << m_name << "\"," << actionString << ");" << std::endl;
              break;
          case FIXED:
              os << lhs << " = new ParserKeyword(\"" << m_name << "\",(size_t)" << m_fixedSize << "," << actionString << ");" << std::endl;
              break;
          case OTHER:
              if (isTableCollection())
                  os << lhs << " = new ParserKeyword(\"" << m_name << "\",\"" << m_sizeDefinitionPair.first << "\",\"" << m_sizeDefinitionPair.second << "\"," << actionString << ", true);" << std::endl;
              else
                  os << lhs << " = new ParserKeyword(\"" << m_name << "\",\"" << m_sizeDefinitionPair.first << "\",\"" << m_sizeDefinitionPair.second << "\"," << actionString << ");" << std::endl;
              break;
          }
      }
      
      for (size_t i = 0; i < m_record->size(); i++) {
          os << indent << "{" << std::endl;
          {
              const std::string local_indent = indent + "   ";
              ParserItemConstPtr item = m_record->get(i);
              os << local_indent << "ParserItemConstPtr item(";
              item->inlineNew(os);
              os << ");" << std::endl;
              {
                  std::string addItemMethod = "addItem";
                  if (m_isDataKeyword)
                      addItemMethod = "addDataItem";
                  
                  os << local_indent << lhs << "->" << addItemMethod << "(item);" << std::endl;
              }
          }
          os << indent << "}" << std::endl;
      }
  }
}
