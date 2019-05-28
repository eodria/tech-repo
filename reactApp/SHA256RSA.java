import java.net.URLEncoder;
import java.nio.charset.StandardCharsets;
import java.security.KeyFactory;
import java.security.MessageDigest;
import java.security.Signature;
import java.security.spec.PKCS8EncodedKeySpec;
import java.util.Base64;

public class SHA256RSA {
    
	public static void main(String[] args) throws Exception {
		String consumerKey = "kFYFfq2AdNISlXyNVIKLXIdfb8O-U-vi3eOWcjH62333babd!7e70de1fb6e84f63bc17fef6f34f4f320000000000000000";
		String encodedKey  = "kFYFfq2AdNISlXyNVIKLXIdfb8O-U-vi3eOWcjH62333babd%217e70de1fb6e84f63bc17fef6f34f4f320000000000000000";
		String nonce = "1111111111111111118";
		long seconds = System.currentTimeMillis() / 1000l;
		String timestamp = Long.toString(seconds);
		String method = "POST";
		String requestUrl = "https%3A%2F%2Fsandbox.api.mastercard.com%2Fsend%2Fv1%2Fpartners%2Fptnr_H8RWWeKXU0nWun6nKWZcLAotVCt%2Fconsumers";
		String requestBody = 
		 "<consumer><consumer_reference>AB123456</consumer_reference><first_name></first_name><middle_name>Tyler</middle_name><last_name>Smith</last_name><nationality>USA</nationality><date_of_birth>1999-12-30</date_of_birth><address><line1>1 Main St</line1><line2>Apartment 9</line2><city>OFallon</city><country_subdivision>MO</country_subdivision><postal_code>63368</postal_code><country>USA</country></address><primary_phone>11234567890</primary_phone><primary_email>Jane.Smith123@abcmail.com</primary_email></consumer>";
		MessageDigest digest = MessageDigest.getInstance("SHA-256");
		byte[] bytes = digest.digest(requestBody.getBytes(StandardCharsets.UTF_8));
		String hash = Base64.getEncoder().encodeToString(bytes);
		String encodedHash = URLEncoder.encode(hash, "UTF-8");
		System.out.println("hash: " + hash);
		System.out.println("encodedHash: " + encodedHash);
		String baseString = 
				  method + "&amp;" + requestUrl + "&amp;oauth_body_hash%3D" + encodedHash
				  + "%26oauth_consumer_key%3D" + encodedKey + "%26oauth_nonce%3D" + nonce 
				  + "%26oauth_signature_method%3DRSA-SHA256%26oauth_timestamp%3D" + timestamp + "%26oauth_version%3D1.0";
		
		System.out.println("baseString: " + baseString);
		
		String input = baseString;
		String strPk = "-----BEGIN PRIVATE KEY-----\n" + 
				"MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQChlQNClPoxOpbz\n" + 
				"d7kAlQeD0gNpUk0gxyInkR+32FX/7X2kQZxGcz8YSJn7/8V8GyOdVQ+bFO/8X7T1\n" + 
				"dUjriQY2gbCzVLem2o20mxerYbyZs8lJZYMorhED7LPiQJCxRJksyVOEt8ALD/RM\n" + 
				"Frrq/ryjL59DV3GfbbA5p577aYmc83Qh7XzYFyhDoxmyis06T3jBYVyluhUIcYPA\n" + 
				"NLm+Yo1Ux/GqyCCqbGBaH9apN0JLZ/5c6nxuuA3dZMMwVzoGEjXXL2Vq9WgPur0q\n" + 
				"VI83zcwoBssvRSXq9X0zQ4MLoeNiYouUxDGPNNLV0udUl815XWjZ6iPC61fAIiTp\n" + 
				"tBLz6adzAgMBAAECggEAL/gM/7c6j2LVbXIdeg7ELF4spRfJzfFQMm7i+7E91ryq\n" + 
				"kBjqS//qu0rSa/2Co1VrPUoDWAbfOA0xq60l+Ma1EXlu+84qt14qs3NXk4lwdDDr\n" + 
				"kN6SEYGbi4652HFSg03aK3GZWc0zNhWGcRBObXUyiz+YCGXtVWc66eLZT8oFEJK6\n" + 
				"rtXalcOFuJg67ic04/6MqafxCvHi3yjqaejskkdG+c9AwNdi0/PsyTMB+FjgXuTH\n" + 
				"LDBGfn8rZsHQa63Eb1WblAWFAtTIVOZJzDHjmYIN9e9PE5KW3593FybCG3s54wlt\n" + 
				"SGsFthWhE7j5ouxOL/b+rwz2QSqr9nAZ72vLqzLgAQKBgQD4zb4pj4xPVLCN1pse\n" + 
				"zrmntURwfVb4MBeJBduePXmQBlksPXoTfW67o/Ty5pNUdHY6ym7c96zwNVJD7SEl\n" + 
				"83B9QnBJPvAeH3129S+DeIR/gN2PkFczqhqEQdtJ4djTbw01LcqgsCxsnurciX8M\n" + 
				"6A+rFnNqgk32REcYsi9XmobmnwKBgQCmQXDitZ6yP2U8ngNuRdsSM9eEwFk/JauJ\n" + 
				"o1tWg5ioPUxRATKigA9RYYnmjusuyj68nLNgiKcyNcsIxOtXDuuuxSczpBoxz6A9\n" + 
				"H06Fs9fwIHXMbM1jRuCRfM3JIj3VcnKYsZo42039P9On5mMNq1c7lCcTK/iTkxC6\n" + 
				"Hi4PQdNyrQKBgB5k0+Qz2wod6GuLWevLiTaJmFC9nxA2n8lnY8rS67TCccZLxEv5\n" + 
				"OLvHbbb2n+zjRUYtUJnBz+iQOvp9T11ZfdZvdVQdUiv74NHVZfaDlHwW+/n7wgtX\n" + 
				"rmGVwfomktxRZqWmcGhs3SBv0XUbft/OwYs+9N6ACJIbCSxlKA9lmzE1AoGAE84R\n" + 
				"43/dMsCoj6sHH5jEuwtf7hl3yDT1lUTXeZvdkJUUxUWOQZtdt8UVx1hZHc7L4G4O\n" + 
				"VD6TJEESOa4zlsBZRQ2JWPaAhFSGcyGKmhTNcXZ8cei4Is5rrAkg/ofMs+OZUSU/\n" + 
				"COzfAn8a+JJSVGuHzbodWYRQbUwEnxc/R2QBC1UCgYEAuhZg1WaFg1g2NDGnBCKj\n" + 
				"bZTuXpJT//UP9cy6ZduxEHOCx7UoHUfBHaHTCTEgEVpp8kzzw6//Lwo3RtJFm2tE\n" + 
				"c5Bjjq0T1e94bXOLafZrUBMpvvOaib1VJIemKEdIdCa3xWqz3Gy1OJoimKekrL1F\n" + 
				"S26snMVe+1anJdCb6dDMV30=\n" + 
				"-----END PRIVATE KEY-----";
		String base64Signature = signSHA256RSA(input, strPk);
		
		String encodedSignature = URLEncoder.encode(base64Signature, "UTF-8");
	    System.out.println("RSA signature: " + encodedSignature); // this is the RSA signature
	    
	    String header = 
	    		 "oauth_body_hash=\"" + hash + "\",oauth_nonce=\"" + nonce + "\",oauth_signature=\"" + encodedSignature 
	    		 + "\",oauth_consumer_key=\"" + consumerKey  
	    		 + "\",oauth_signature_method=\"RSA-SHA256\",oauth_timestamp=\"" + timestamp + "\",oauth_version=\"1.0\"";
	    
	    System.out.println("header: " + header);
	   
	}
	
	private static String signSHA256RSA(String input, String strPk) throws Exception {
		
		String realPK = strPk.replaceAll("-----END PRIVATE KEY-----", "")
                .replaceAll("-----BEGIN PRIVATE KEY-----", "")
                .replaceAll("\n", "");

        byte[] b1 = Base64.getDecoder().decode(realPK);
        PKCS8EncodedKeySpec spec = new PKCS8EncodedKeySpec(b1);
        KeyFactory kf = KeyFactory.getInstance("RSA");

        Signature privateSignature = Signature.getInstance("SHA256withRSA");
        privateSignature.initSign(kf.generatePrivate(spec));
        privateSignature.update(input.getBytes("UTF-8"));
        byte[] s = privateSignature.sign();
        
        String result = Base64.getEncoder().encodeToString(s);
        return result;
	}
}