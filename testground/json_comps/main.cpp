#include <string>
#include <unordered_map>
#include <fstream>
#include <any>
#include <iostream>
#include <vector>
#include "cereal/cereal.hpp"
#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/archives/binary.hpp>

#define CEREAL_FUTURE_EXPERIMENTAL
#include <cereal/archives/adapters.hpp>
#include "cereal/types/unordered_map.hpp"
#include "cereal/types/vector.hpp"
#include "cereal/types/string.hpp"

template<typename Traits>
  struct MappedArchetype {
  //typename Traits::ManagerType& cm; 
  std::unordered_map<typename Traits::FieldIdentifier,typename Traits::FieldType> fields;
  std::vector<typename Traits::FieldIdentifier> hashable;

    std::string simple_format;
    std::string repr_format;

  //  MappedArchetype(typename Traits::ManagerType& m, const std::unordered_map<typename Traits::FieldIdentifier,typename Traits::FieldType>& f, const std::vector<typename Traits::FieldIdentifier>& h): cm{m}, fields{f}, hashable{h}{}
    MappedArchetype(const std::unordered_map<typename Traits::FieldIdentifier,typename Traits::FieldType>& f, const std::vector<typename Traits::FieldIdentifier>& h): fields{f}, hashable{h}{}
    MappedArchetype(const MappedArchetype&) =default;
    MappedArchetype() =default;


    template<typename Archive>
    void serialize(Archive& ar){
      ar(
         cereal::make_nvp("Fields", fields),
         cereal::make_nvp("Hashable Fields" , hashable),
         cereal::make_nvp("SimpleFormat", simple_format),
         cereal::make_nvp("ReprFormat", repr_format)
         );
    }
  };


namespace cereal
  {
  template <class Archive, class C, class A, class D,
  traits::EnableIf<traits::is_text_archive<Archive>::value> = traits::sfinae> inline
  void save( Archive & ar, std::unordered_map<std::string, D, C, A> const & map )
  {
  for( const auto & i : map )
  ar( cereal::make_nvp( i.first, i.second ) );
  }

  template <class Archive, class C, class A, class D,
  traits::EnableIf<traits::is_text_archive<Archive>::value> = traits::sfinae> inline
  void load( Archive & ar, std::unordered_map<std::string, D, C, A> & map )
  {
  map.clear();

  auto hint = map.begin();
  while( true )
  {
  const auto namePtr = ar.getNodeName();

  if( !namePtr )
  break;

  std::string key = namePtr;
  D value; ar( value );
  hint = map.emplace_hint( hint, std::move( key ), std::move( value ) );
  }
  }
  } 


// Class which can serialize a map if std::any data based on an archetype
struct AnyMap {
  const std::unordered_map<std::string, std::string>& archetype;
  std::unordered_map<std::string, std::any> values;

  AnyMap( const std::unordered_map<std::string, std::string>& v): archetype{v}{}

  template<typename Archive>
  void save(Archive& ar) const {
    for(const auto& vals: archetype){
#define ArchiveValue(TYPESTRING, TYPE)                      \
      else if(vals.second == TYPESTRING){                         \
        auto cast_val = std::any_cast<TYPE>(values.at(vals.first)); \
        ar(cereal::make_nvp(vals.first, cast_val));         \
      }
      if(false){}
      ArchiveValue("str", std::string)
      ArchiveValue("int", int)
      ArchiveValue("vint", std::vector<int>)
      ArchiveValue("float", float)
      else {              
        auto cast_val = std::any_cast<int>(values.at(vals.first)); 
        ar(cereal::make_nvp(vals.first, cast_val));         
      }
    }
  }

  template<typename Archive>
  void load(Archive& ar) {
#define UnArchiveValue(TYPESTRING, TYPE)                      \
    else if(vals.second == TYPESTRING) {                    \
        TYPE cast_val; \
        ar(cereal::make_nvp(vals.first, cast_val));         \
        values[vals.first] = cast_val;                      \
      }

    for(const auto& vals: archetype){
      if(false){}
      UnArchiveValue("str", std::string)
      UnArchiveValue("int", int)
      UnArchiveValue("vint", std::vector<int>)
      UnArchiveValue("float", float)
      else {              
        int comp;
        ar(cereal::make_nvp(vals.first, comp));         
        values[vals.first] = comp;
      }
    }
  }
};


class ComponentManager;

struct ComponentManagerTraits{
  using ManagerType = ComponentManager;
  using ArchetypeHandle = std::string;
  using ComponentHandle = int;
  using FieldIdentifier = std::string;
  using FieldType = std::string;
  using Field=std::any;
  using KeyValues= std::unordered_map<FieldIdentifier,Field>;
};


class ComponentManager{
  using ArchetypeHandle = std::string;
  using ComponentHandle = int;
  using FieldIdentifier = std::string;
  using FieldType = std::string;
  using Field=std::any;
  using KeyValues= std::unordered_map<FieldIdentifier,Field>;
  using InputArchiveType = cereal::JSONInputArchive ;
  using OutputArchiveType= cereal::JSONOutputArchive ;

  ComponentHandle comp_next = 0;


public:
  using Archetype = MappedArchetype<ComponentManagerTraits>;

  struct ComponentData {
    ArchetypeHandle archetype;
    KeyValues data;
  };

  class Component{
    ComponentManager& cm;
    ComponentHandle handle;

  public:

    Component(ComponentManager& c, ComponentHandle h): cm{c}, handle{h}{}
    template<typename T>
    T field(const FieldIdentifier& field) const {
      return cm.getComponentField<T>(handle, field) ;
    }
    /*

    template<typename Archive>
    void save(Archive& ar) const {
      const ComponentData& data = cm.components.at(handle);
      const Archetype& arch = cm.archetypes.at(data.archetype);
      AnyMap to_save(arch.fields);
      to_save.values = data.data;
      ar(cereal::make_nvp("Handle", handle));
      ar(cereal::make_nvp("Archetype", data.archetype));
      ar(cereal::make_nvp("Fields", to_save));
    }

    template<typename Archive>
    static void load_and_construct(Archive& ar, cereal::construct<Component>& construct {
      cm = *cereal::get_user_data<UD>(ar).cm;
      construct(cm, -1);
      ArchetypeHandle arch_handle;
      ar(cereal::make_nvp("Handle", construct->handle));
      ar(cereal::make_nvp("Archetype", arch_handle));
      const Archetype& arch = cm.archetypes.at(arch_handle);
      AnyMap to_save(arch.fields);
      ar(cereal::make_nvp("Fields", to_save));
      cm.createComponent(arch_handle, handle, to_save.values);
    }
    */
  };


  template<typename T>
  T getComponentField(ComponentHandle comp_handle, const FieldIdentifier& field) const {
    const auto& temp = components.at(comp_handle);
    if constexpr(std::is_same_v<std::any,T>){
      return temp.data.at(field);}
      else {
    return std::any_cast<T>(temp.data.at(field));
      }
  }


  std::unordered_map<ArchetypeHandle, Archetype> archetypes;
  std::unordered_map<ComponentHandle, ComponentData> components;

  Component createComponent(const ArchetypeHandle& arch, KeyValues d){
    components[comp_next] = {arch, d};
    ++comp_next;
    return Component(*this, comp_next);
  }

  Component createComponent(const ArchetypeHandle& arch, ComponentHandle h, KeyValues d){
    components[h] = {arch, d};
    return Component(*this, h);
  }

  template<typename T>
  void saveArchetypes(T& x){
    OutputArchiveType archive( x );
    archive(cereal::make_nvp("Archetypes", archetypes));
  }

  template<typename T>
  void loadArchetypes(T& x){
    InputArchiveType archive( x );
    archive(cereal::make_nvp("Archetypes", archetypes));
  }


  struct UD{
    ComponentManager* cm;
  };

  struct SaveHelper{
    ComponentManager* cm;
    ComponentData d;
    ComponentHandle h;
  

    template<typename Archive>
    void serialize(Archive& ar){
      cm = cereal::get_user_data<UD>(ar).cm;
      ar(cereal::make_nvp("Handle", h));
      ar(cereal::make_nvp("Archetype", d.archetype));
      AnyMap to_save(cm->archetypes.at(d.archetype).fields);
      to_save.values = d.data;
      ar(cereal::make_nvp("Data" , to_save));
      d.data=std::move(to_save.values);
    }
  };


  template<typename T>
  void saveComponents(T& x){
    UD ud{this};
    cereal::UserDataAdapter<UD, OutputArchiveType> archive(ud, x);
    std::vector<SaveHelper> temp;
    for(const auto& comp : components){
      SaveHelper s{this, comp.second, comp.first};
      temp.push_back(s);
    }
    archive(temp);
  }

  template<typename T>
  void loadComponents(T& x){
    UD ud{this};
        cereal::UserDataAdapter<UD, InputArchiveType> archive(ud, x);
        std::vector<SaveHelper> temp;
        archive(temp);
        for(SaveHelper& sh: temp){
          components[sh.h] = std::move(sh.d);
        }

  }
};



int main(){

  ComponentManager cm;
    std::ifstream ifsa("archs.json");
    cm.loadArchetypes(ifsa);
    cm.saveArchetypes(std::cout);
    std::ifstream ifs("comps.json");
    cm.loadComponents(ifs);
    cm.saveComponents(std::cout);
  //  cm.archetypes.insert(std::pair<std::string, ComponentManager::Archetype>("Module", ComponentManager::Archetype(cm, {{"U" ,"int"},{ "V", "int"}}, {"V"})));
  //  cm.archetypes.insert(std::pair<std::string, ComponentManager::Archetype>("Engine", ComponentManager::Archetype(cm, {{"Left" ,"Module"},{ "Right", "Module"}}, {"Left","Right"})));
  //    cm.createComponent("Module", {{"U", 1}, {"V" ,2} , {"Plane", 4}});
  //    cm.createComponent("Module", {{"U", 4}, {"V" ,3} , {"Plane", 41}});
  //    cm.createComponent("Engine", {{"Left", 1}, {"Right" ,2}});
    return 0;
}
