# Phenodata Structure Documentation: AVL-Trie Hybrid for Government ID Systems

## Overview: Phenodata Structure

The **Phenodata Structure** is a hybrid data architecture that combines AVL tree balancing properties with Trie character-level indexing to create a robust system for storing government-issued identification data with subjective context preservation.

### Core Concept

A phenodata node represents a single atomic unit of information that can store:
- **Primitive types**: char, int, bool
- **Strong/Weak typed values**: Dynamic or statically typed data
- **Phenomenological context**: Subjective frame of reference data

## Architecture Components

### 1. AVL-Trie Hybrid Structure

```rust
// Phenodata node combining AVL balancing with Trie character storage
pub struct PhenodataNode<T: Ord + Copy> {
    // Core data
    pub value: T,                    // Char/Int/Bool primitive
    pub node_type: DataType,         // Strong/Weak typing info
    
    // AVL properties for balance
    pub height: i32,
    pub left: Option<Box<PhenodataNode<T>>>,
    pub right: Option<Box<PhenodataNode<T>>>,
    
    // Trie properties for character-level indexing
    pub children: HashMap<char, Box<PhenodataNode<T>>>,
    pub is_terminal: bool,
    
    // Phenomenological context
    pub phenomenohog: Option<PhenomenohogBlock>,
}
```

### 2. Frame of Reference for Government IDs

The frame of reference specifically handles government-issued identification:

```rust
pub struct GovernmentIDFrame {
    pub id_type: IDType,              // NI, SSN, Birth Certificate
    pub issuing_authority: String,    // Country/State
    pub validation_status: bool,      // Cryptographically verified
    pub phenomenohog_context: PhenomenohogBlock,
}

pub enum IDType {
    NationalInsurance(String),        // UK NI: AB123456C
    SocialSecurity(String),          // US SSN: 123-45-6789
    BirthCertificate {
        number: String,
        district: String,
        year: u32,
    },
    PassportNumber(String),
    DriverLicense(String),
}
```

### 3. Phenomenohog Subject Context

The phenomenohog block captures person-to-person context without AI interaction:

```rust
pub struct PhenomenohogBlock {
    // Subject identification
    pub session: String,              // Unique session ID
    pub scope: String,               // "person" | "instance" | "context"
    pub type_field: String,          // "government_id" | "biometric" | "preference"
    
    // Frame of reference (person-to-person)
    pub frame_of_reference: String,   // "subject:john_doe,verifier:jane_smith,id_type:ni"
    
    // Temporal context
    pub timestamp: DateTime<Utc>,
    
    // Data integrity
    pub diram_state: Diram,          // Null | Partial | Collapse | Intact
}
```

## AVL-Trie Operations for Phenological Memory Model

### Token Type System

```rust
pub enum TokenType {
    // Primitive tokens
    CharToken(char),
    IntToken(i32),
    BoolToken(bool),
    
    // Composite tokens for IDs
    NIToken {
        prefix: [char; 2],
        numbers: [u8; 6],
        suffix: char,
    },
    SSNToken {
        area: u16,      // 001-899
        group: u8,      // 01-99
        serial: u16,    // 0001-9999
    },
}

pub struct TokenValue {
    pub token_type: TokenType,
    pub raw_value: Vec<u8>,
    pub encoded_value: Vec<u8>,    // Cryptographic representation
    pub memory_weight: f32,         // For pruning decisions
}
```

### Balanced Rotation for Phenological Memory

The AVL rotations maintain balance while preserving phenomenological context:

```rust
impl<T: Ord + Copy> PhenodataNode<T> {
    // LL Rotation with context preservation
    fn rotate_right_with_context(mut y: Box<PhenodataNode<T>>) -> Box<PhenodataNode<T>> {
        let mut x = y.left.take().unwrap();
        
        // Preserve phenomenohog context during rotation
        if let Some(y_context) = &y.phenomenohog {
            if let Some(x_context) = &mut x.phenomenohog {
                x_context.frame_of_reference.push_str(&format!(
                    ",rotation:LL,parent_context:{}",
                    y_context.session
                ));
            }
        }
        
        y.left = x.right.take();
        y.update_height();
        x.right = Some(y);
        x.update_height();
        x
    }
}
```

## GoSilang Integration Pattern

For integration with the gosilang toolchain:

```go
// Gosilang phenodata structure
type PhenodataNode struct {
    Value       interface{}          // Dynamic typing support
    NodeType    string              // "char" | "int" | "bool"
    Height      int32
    Left        *PhenodataNode
    Right       *PhenodataNode
    Children    map[rune]*PhenodataNode
    IsTerminal  bool
    Phenomenohog *PhenomenohogBlock
}

// Token memory trie for classic span
type TokenMemoryTrie struct {
    Root        *PhenodataNode
    TokenCache  map[string]*TokenValue
    SpanMarkers []SpanMarker  // For taum-like germ data
}

type SpanMarker struct {
    StartPos    int
    EndPos      int
    TokenType   string
    GermData    []byte  // Compressed phenomenological data
}
```

## Riftbridge Integration

For riftbridge compatibility:

```rust
// Riftbridge adapter for phenodata
pub struct RiftbridgeAdapter {
    pub phenodata_root: Box<PhenodataNode<char>>,
    pub span_registry: HashMap<String, SpanMarker>,
    pub germ_data_cache: Vec<u8>,  // Compressed phenomenological patterns
}

impl RiftbridgeAdapter {
    pub fn export_to_riftbridge(&self) -> RiftbridgePayload {
        RiftbridgePayload {
            node_data: self.serialize_phenodata(),
            span_data: self.serialize_spans(),
            germ_patterns: self.germ_data_cache.clone(),
        }
    }
}
```

## Use Case: National Insurance Number Validation

```rust
// Example: Storing and validating UK NI number with full context
let mut phenodata_tree = PhenodataNode::new_root();

let ni_frame = GovernmentIDFrame {
    id_type: IDType::NationalInsurance("AB123456C".to_string()),
    issuing_authority: "HMRC_UK".to_string(),
    validation_status: true,
    phenomenohog_context: PhenomenohogBlock {
        session: "validation_session_001".to_string(),
        scope: "person".to_string(),
        type_field: "government_id".to_string(),
        frame_of_reference: "subject:john_doe,verifier:hmrc_system,context:employment_verification".to_string(),
        timestamp: Utc::now(),
        diram_state: Diram::Intact,
    },
};

// Insert with AVL balancing
phenodata_tree.insert_with_frame("AB123456C", ni_frame);
```

## Summary

The Phenodata Structure provides:
1. **Atomic data units** combining primitives with context
2. **AVL balancing** for O(log n) operations
3. **Trie indexing** for character-level search
4. **Frame of reference** for government ID validation
5. **Phenomenological context** preservation without AI mediation
6. **Cryptographic integrity** through DIRAM states

This architecture ensures that government-issued identifications are stored with their full subjective context while maintaining efficient access patterns and data integrity.
