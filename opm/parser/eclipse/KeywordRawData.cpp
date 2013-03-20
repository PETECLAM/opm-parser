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

#include <utility>
#include "KeywordRawData.hpp"
namespace Opm {

    KeywordRawData::KeywordRawData() {
    }

    void KeywordRawData::addKeywordDataBlob(const std::string& keyword, const std::list<std::string>& blob) {
        m_keywordRawData.push_back(std::make_pair(keyword, blob));
    }

    int KeywordRawData::numberOfKeywords() {
        return m_keywordRawData.size();
    }

    void KeywordRawData::getListOfKeywords(std::list<std::string>& keywords) {
        keywords.clear();
        std::list< std::pair< std::string, std::list<std::string > > >::iterator iterator;
        for (iterator = m_keywordRawData.begin(); iterator != m_keywordRawData.end(); iterator++) {
            keywords.push_back(iterator->first);
        }
    }

    KeywordRawData::~KeywordRawData() {
    }
}
