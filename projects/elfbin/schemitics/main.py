# main.py - Bringing it all together
from gossip_schema_bridge import GossipSchemaBridge
from pathlib import Path

def build_safe_housing():
    # Load GOSSIP robotics planning code
    gossip_code = Path("robotics_planning/hitl_recovery/hitl_recovery.rp").read_text()
    
    # Initialize bridge
    bridge = GossipSchemaBridge()
    
    # Compile to blueprint
    blueprint = bridge.compile_to_blueprint(gossip_code)
    
    # Generate PlantUML diagram
    plantuml_diagram = bridge.export_to_plantuml(blueprint)
    
    # Save for construction documentation
    with open("housing_blueprint.puml", "w") as f:
        f.write(plantuml_diagram)
    
    # Validate IWU compliance
    print(f"IWU Compliant: {bridge.schema_lang.validate_iwu_compliance(blueprint)}")
    
    # Generate construction documents
    print(json.dumps(blueprint, indent=2))

if __name__ == "__main__":
    build_safe_housing()
