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

#ifndef DECKITEM_HPP
#define DECKITEM_HPP

#include <stdexcept>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

namespace Opm {

    class DeckItem {
    public:
        DeckItem(const std::string& name);
        const std::string& name() const;
        
        bool defaultApplied() const;

        virtual size_t size() const = 0;
        
        virtual int getInt(size_t index) const {
            throw std::logic_error("This implementation of DeckItem does not support int");
        };

        virtual double getDouble(size_t index) const {
            throw std::logic_error("This implementation of DeckItem does not support double");
        };

        virtual bool getBool(size_t index) const {
            throw std::logic_error("This implementation of DeckItem does not support bool");
        }

        virtual std::string getString(size_t index) const {
            throw std::logic_error("This implementation of DeckItem does not support string");
        }

        

        virtual const std::vector<int>& getIntData( ) const {
            throw std::logic_error("This implementation of DeckItem does not support int");
        };

        virtual const std::vector<double>& getDoubleData() const {
            throw std::logic_error("This implementation of DeckItem does not support double");
        };

        virtual const std::vector<std::string>& getStringData() const {
            throw std::logic_error("This implementation of DeckItem does not support string");
        }



        virtual ~DeckItem() {
        }

        bool m_defaultApplied;
    private:
        std::string m_name;
    };

    typedef boost::shared_ptr<DeckItem> DeckItemPtr;
    typedef boost::shared_ptr<const DeckItem> DeckItemConstPtr;
}
#endif  /* DECKITEM_HPP */

